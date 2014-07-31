#include "kelocalclient.h"

#include "talk/base/logging.h"
#include "talk/base/json.h"
#include "talk/base/bind.h"

namespace kaerp2p{

LocalTerminal::LocalTerminal():
    broadcastSocket(NULL),localSocket(NULL),ownThread(false)
{
   //socket的执行必须和socket创建在一个线程,否则socket消息接收不到,
  //若需使用不同线程,可以考虑使用proxy
 //   this->socketThread_ = talk_base::Thread::Current();
//    if(thread == NULL)
    this->socketThread_ = new talk_base::Thread();
    this->socketThread_->Start();
    //socket_factory_.reset(new talk_base::BasicPacketSocketFactory(socketThread_));
}

LocalTerminal::~LocalTerminal()
{
    if(broadcastSocket){
        delete broadcastSocket;
    }
    if(localSocket){
        localSocket->Close();
        delete localSocket;
    }
    delete this->socketThread_;
}

bool LocalTerminal::Initialize()
{
    return socketThread_->Invoke<bool>(
                talk_base::Bind(&LocalTerminal::Initialize_s,this));
}

bool LocalTerminal::OpenTunnel(const std::string &peerAddr)
{
    return socketThread_->Invoke<bool>(
                talk_base::Bind(&LocalTerminal::OpenTunnel_s,this,peerAddr));
}

bool LocalTerminal::CloseTunnel(const std::string &peerAddr)
{
    LOG(INFO)<<"LocalTerminal::CloseTunnel";
    if (localSocket) {
        localSocket->Close();
        delete localSocket;
        localSocket = NULL;
        this->SignalTunnelClosed(this,peerAddr);
    }
    return true;
}

bool LocalTerminal::SendByRouter(const std::string &peerAddr, const std::string &data)
{
    return SendByRouter(peerAddr,data.c_str(),data.size());
}

bool LocalTerminal::SendByRouter(const std::string &peerAddr, const char *data, size_t len)
{
    talk_base::SocketAddress addr;
    addr.FromString(peerAddr);
    talk_base::PacketOptions options;
    int ret = broadcastSocket->SendTo(data,len,addr,options);
    LOG(INFO)<<"LocalTerminal::SendByRouter--ret="<<ret;
    if (ret < 0 ) {
        LOG(INFO)<<"LocalTerminal::SendByRouter--send error "<<broadcastSocket->GetError();
        return false;
    }
    return true;
}

bool LocalTerminal::SendByTunnel(const std::string &peerAddr, const std::string &data)
{
    return SendByTunnel(peerAddr,data.c_str(),data.size());
}

bool LocalTerminal::SendByTunnel(const std::string &peerAddr, const char *data, size_t len)
{
    talk_base::PacketOptions options;
    int ret = localSocket->Send(data,len,options);
    if(ret < 0){
        LOG(INFO)<<"LocalTerminal::SendByTunnel--send error "<<
                   localSocket->GetError();
        return false;
    }
    return true;
}
const int kLocalSocketStartPort = 22555;
const int kLocalSocketEndPort = 22600;

bool LocalTerminal::Initialize_s()
{
    int port = 0;
    talk_base::SocketAddress int_addr(INADDR_ANY,port);
    broadcastSocket = talk_base::AsyncUDPSocket::Create(
                this->socketThread_->socketserver(),int_addr);
    if(!broadcastSocket) {
        LOG(WARNING) << "Failed to create a UDP socket bound at port"<< port;
        int_addr.SetPort(++port);
        broadcastSocket = talk_base::AsyncUDPSocket::Create(
                    this->socketThread_->socketserver(),int_addr);
    }
    if(broadcastSocket){
        broadcastSocket->SignalReadPacket.connect(this,&LocalTerminal::OnPackage);
        int ret = broadcastSocket->SetOption(talk_base::Socket::OPT_BROADCAST,1);
        LOG(INFO)<<"LocalTerminal::Initialize--- set opt "<<ret;
    }else{
        LOG(WARNING) << "Failed to create again UDP socket bound at port"<< port;
        return false;

    }
    return true;
}

bool LocalTerminal::OpenTunnel_s(const std::string &peerAddr)
{
    LOG(INFO)<<"LocalTerminal::OpenTunnel";
    talk_base::SocketAddress bindAddr(INADDR_ANY,0);
    talk_base::SocketAddress remoteAddr;
    remoteAddr.FromString(peerAddr);
    localSocket = talk_base::AsyncRawTCPSocket::Create(
                socketThread_->socketserver(),bindAddr,remoteAddr);
    if(!localSocket){
        LOG(WARNING) << "Failed to create a tcp socket to "<<peerAddr;
        return false;
    }
    LOG(INFO)<<"connection state "<<localSocket->GetState();
    localSocket->SignalConnect.connect(this,&LocalTerminal::OnLocalTcpConnect);
    localSocket->SignalClose.connect(this,&LocalTerminal::OnLocalTcpClose);
    localSocket->SignalReadPacket.connect(this,&LocalTerminal::OnPackage);
    localSocket->SetOption(talk_base::Socket::OPT_NODELAY,1);
    return true;
}

void LocalTerminal::OnLocalTcpConnect(talk_base::AsyncPacketSocket *socket)
{
    std::string peerAddr = socket->GetRemoteAddress().ToString();
    LOG(INFO)<<"LocalTerminal::LocalTcpConnect---"<< peerAddr<<
               " connection state "<<localSocket->GetState();
    this->SignalTunnelOpened(this,peerAddr);
}

void LocalTerminal::OnLocalTcpClose(talk_base::AsyncPacketSocket *socket, int err)
{
    std::string peerAddr = socket->GetRemoteAddress().ToString();
    LOG(INFO)<<"LocalTerminal::OnLocalTcpClose---"<<err<<" peerAddr:"<<peerAddr;
    this->CloseTunnel(peerAddr);
    //this->SignalTunnelClosed(this,peerAddr);
}

void LocalTerminal::OnPackage(
        talk_base::AsyncPacketSocket *socket, const char *buf, size_t size,
        const talk_base::SocketAddress &remote_addr,
        const talk_base::PacketTime &packet_time)
{
    std::string peerAddr = remote_addr.ToString();
    talk_base::Buffer buffer(buf,size);
    if(socket == broadcastSocket){
        this->SignalRouterMessage(peerAddr,buffer);
    }
    if(socket == localSocket){
        this->SignalTunnelMessage(peerAddr,buffer);
    }
}

KeLocalClient::KeLocalClient()
{
}

bool KeLocalClient::Init(PeerTerminalInterface *t)
{
    KeMsgProcessContainer::Init(t);
    //this->has_terminal = true;

    broadcastMsg.reset(new KeLocalMessage("broadcast",this));
    broadcastMsg->SignalNeedSendData.connect(
                (KeMsgProcessContainer*)this,
                &KeMsgProcessContainer::SendSelfData);
    broadcastMsg->SignalNetDeviceInfo.connect(
                this,&KeLocalClient::OnSearchedDeviceInfo);

    return true;
}
bool KeLocalClient::StartPeerMedia(std::string peerAddr)
{
    KeLocalMessage * process =
            dynamic_cast<KeLocalMessage *>( this->GetProcess(peerAddr));
    if(process == NULL){
        LOG(WARNING) << "process not found "<<peerAddr;
        return false;
    }
    //video : 0-start ,1 - stop
    process->AskMedia(0,0,1);
    return true;
}

bool KeLocalClient::StopPeerMedia(std::string peerAddr)
{
    KeMessageProcessClient * process =
            dynamic_cast<KeMessageProcessClient *>(this->GetProcess(peerAddr));
    if (process == NULL) {
        LOG(WARNING) << "process not found "<<peerAddr;
        return false;
    }
    process->AskVideo(1,1,1);
    return true;
}

bool KeLocalClient::Init(PeerConnectionClientInterface *client)
{
    return false;
}

void KeLocalClient::OnTunnelOpened(PeerTerminalInterface *t, const std::string &peerAddr)
{
    LOG(INFO)<<"KeLocalClient::OnTunnelOpened"<<peerAddr;
    ASSERT(terminal_ == t);
    KeLocalMessage *process = new KeLocalMessage(peerAddr,this);
    process->SignalRecvAudioData.connect(this,&KeLocalClient::OnRecvAudioData);
    process->SignalRecvVideoData.connect(this,&KeLocalClient::OnRecvVideoData);

    this->AddMsgProcess(process);
}

void KeLocalClient::OnRouterMessage(const std::string &peer_id, talk_base::Buffer &msg)
{
    LOG(INFO)<<"KeLocalClient::OnRouterMessage---"<<peer_id<<" length="<<msg.length();
    broadcastMsg->OnProcessMessage(peer_id,msg);
}

bool KeLocalClient::SearchLocalDevice()
{
    talk_base::Buffer sendData;
    int msgLen = sizeof(KESearchOnlineDeviceReq);
    sendData.SetLength(msgLen);
    KESearchOnlineDeviceReq * pMsg = (KESearchOnlineDeviceReq *)sendData.data();
    pMsg->protocal = PROTOCOL_HEAD;
    pMsg->msgType = KEDevMsg_SearchOnlineDevice;
    pMsg->msgLength = msgLen;
    memcpy(pMsg->kaer,"KAER970326",10);
    return terminal_->SendByRouter("255.255.255.255:22616",
                                   sendData.data(),sendData.length());
}

void KeLocalClient::OnSearchedDeviceInfo(const std::string &devInfo)
{
    LOG(INFO)<<"KeLocalClient::OnSearchedDeviceInfo---"<<devInfo;
}

void KeLocalClient::OnRecvAudioData(const std::string &peer_id, const char *data, int len)
{

}

void KeLocalClient::OnRecvVideoData(const std::string &peer_id, const char *data, int len)
{

}


KeLocalMessage::KeLocalMessage(std::string peerAddr, KeLocalClient *container):
    KeMsgProcess(peerAddr,container)
{
    this->heartMissStop = 3;
    this->heartSendDelay = 3000;
}

void KeLocalMessage::AskMedia(int video, int listen, int talk)
{
    talk_base::Buffer sendBuf;
    int msgLen = sizeof(KEVideoServerReq);
    sendBuf.SetLength(msgLen);
    KEVideoServerReq * pReqMsg;
    pReqMsg = (KEVideoServerReq *)sendBuf.data();
    pReqMsg->protocal = PROTOCOL_HEAD;
    pReqMsg->msgType = KEMSG_TYPE_VIDEOSERVER;
    pReqMsg->msgLength = msgLen;
    pReqMsg->clientID = 0;
    pReqMsg->channelNo = 1;
    pReqMsg->videoID = 1;
    pReqMsg->video = video;
    pReqMsg->listen = listen;
    pReqMsg->talk = talk;
    pReqMsg->protocalType = 0;
    pReqMsg->transSvrIp = 0;

    SignalNeedSendData(this->peer_id(),sendBuf.data(),sendBuf.length());
}


void KeLocalMessage::OnMessageRespond(talk_base::Buffer &msgData)
{
    char msgType = msgData.data()[1];
    switch(msgType){
    case KEDevMsg_SearchOnlineDevice:
        OnRecvSearchOnlineDeviceResp(msgData);
        break;
    case KEMSG_TYPE_VIDEOSERVER:
        RecvAskMediaResp(msgData);
        break;
    case KEMSG_TYPE_MEDIATRANS:
        break;
    case KEMSG_TYPE_AUDIOSTREAM:
        RecvAudioData(msgData);
        break;
    case KEMSG_TYPE_VIDEOSTREAM:
        RecvVideoData(msgData);
        break;
    default:
        KeMsgProcess::OnMessageRespond(msgData);
        break;
    }

}

//std::string GetLittleEndianIp(int srcIp){
//    int netIp = talk_base::HostToNetwork32(srcIp);
//    talk_base::IPAddress addr(netIp);
//    return addr.ToString();
//}

void KeLocalMessage::OnRecvSearchOnlineDeviceResp(talk_base::Buffer &msgData)
{
    KESearchOnlineDeviceResp * pMsg = (KESearchOnlineDeviceResp *)msgData.data();

    Json::StyledWriter writer;
    Json::Value jmessage;
    jmessage["ip"] = GetLittleEndianIp(pMsg->devIp);
    jmessage["mask"] = GetLittleEndianIp(pMsg->devMask);
    jmessage["gateway"] = GetLittleEndianIp(pMsg->devGateWay);
    jmessage["port"] = pMsg->port;
    jmessage["sn"] = pMsg->devSN;
    std::string msg = writer.write(jmessage);

    SignalNetDeviceInfo(msg);
}

void KeLocalMessage::RecvVideoData(talk_base::Buffer &msgData)
{
    const int sendStartPos = 11;
    int mediaDataLen = msgData.length() - sendStartPos;
    SignalRecvVideoData(this->peer_id(),msgData.data() +
                        sendStartPos,mediaDataLen);
}

void KeLocalMessage::RecvAudioData(talk_base::Buffer &msgData)
{
    const int sendStartPos = 11;
    int mediaDataLen = msgData.length() - sendStartPos;
    SignalRecvAudioData(this->peer_id(),msgData.data() +
                        sendStartPos,mediaDataLen);
}

void KeLocalMessage::RecvAskMediaResp(talk_base::Buffer &msgData)
{
    KEVideoServerResp * msg = (KEVideoServerResp *)msgData.data();
    LOG(INFO)<<"KeLocalMessage::RecvAskMediaResp---"<<msg->msgLength;
}

}
