#include "peerterminal.h"
#include "KeMessage.h"
#include "talk/base/json.h"
#include "talk/base/bind.h"

namespace kaerp2p {



PeerTerminal::PeerTerminal(kaerp2p::PeerConnectionClientInterface * client,
                           int max_tunnel):
    client_(client),max_tunnel_num_(max_tunnel),tunnel_out_action(1)
{
    client_->SignalMessageFromPeer.connect(this,&PeerTerminal::OnRouterReadData);
}

bool PeerTerminal::OpenTunnel(const std::string & peer_id)
{
    ScopedTunnel aTunnel = this->GetOrCreateTunnel(peer_id);
    if(aTunnel== NULL){
        LOG(WARNING)<< "not avaliable tunnel";
        return false;
    }
    bool ret = aTunnel->ConnectToPeer(peer_id);
    return ret;
}

bool PeerTerminal::CloseTunnel(const std::string &peer_id)
{
    LOG(INFO)<< "PeerTerminal::CloseTunnel" << peer_id;

    ScopedTunnel aTunnel = this->GetTunnel(peer_id);
    if(aTunnel == NULL){
        LOG(WARNING)<< "PeerTerminal::CloseTunnel-----tunnel not get "<<
                       peer_id;
        return false;
    }
    aTunnel->DisconnectFromCurrentPeer();
    return true;
}


bool PeerTerminal::SendByRouter(const std::string &peer_id,
                                const std::string &data)
{
    return client_->SendToPeer(peer_id,data);
}

bool PeerTerminal::SendByRouter(const std::string &peer_id, const char *data, size_t len)
{
    std::string dataStr(data,len);
    return client_->SendToPeer(peer_id,dataStr);
}

bool PeerTerminal::SendByTunnel(const std::string &peer_id,
                                const std::string &data)
{
    ScopedTunnel aTunnel = this->GetTunnel(peer_id);
    if(aTunnel == NULL){
        return false;
    }
    return aTunnel->GetStreamProcess()->WriteStream(data.c_str(),data.length());
}

bool PeerTerminal::SendByTunnel(const std::string &peer_id,
                                const char *data, size_t len)
{
    ScopedTunnel aTunnel = this->GetTunnel(peer_id);
    if(aTunnel == NULL){
        return false;
    }
    return aTunnel->GetStreamProcess()->WriteStream(data,len);
}


void PeerTerminal::OnTunnelOpened(const std::string &peerId)
{
    ScopedTunnel aTunnel = this->GetTunnel(peerId);
    if(aTunnel == NULL){
        LOG(WARNING)<<"PeerTerminal::OnTunnelOpened---cannot get tunnel by id";
        return ;
    }
    aTunnel->GetStreamProcess()->SignalReadData.connect(
                this,&PeerTerminal::OnTunnelReadData);
    this->SignalTunnelOpened(this,peerId);
}

void PeerTerminal::OnTunnelClosed(const std::string &peerId)
{
    this->SignalTunnelClosed(this,peerId);
}

void PeerTerminal::OnTunnelReadData(kaerp2p::StreamProcess *stream,
                                    talk_base::Buffer& buffer)
{
    //ASSERT(tunnel == conductor_->GetStreamProcess());
    ScopedTunnel aTunnel = this->GetTunnel(stream);
    if(aTunnel == NULL){
        LOG_T_F(WARNING)<<"cannot get tunnel by stream";
        return ;
    }

//    char * buffer = new char[len];
//    size_t readLen;
//    bool result = stream->ReadStream(buffer,len,&readLen);
//    if(!result){
//        LOG_T_F(WARNING)<<"--read stream error";
//        return ;
//    }
//    talk_base::Buffer data(buffer,readLen);
//    delete buffer;
    this->SignalTunnelMessage(aTunnel->GetPeerID(),buffer);
}
/**
 * @brief PeerTerminal::OnRouterReadData,we process message received by client_,
 *        just process the "p2p" type message and the "tunnel" type message
 *        the "p2p" type message is handled by PeerTerminal itself and
 *          the tunnel type message is emit SignalRouterMessage
 * @param peer_id
 * @param msg --- the message received by client_, the message should be a json string
 */
void PeerTerminal::OnRouterReadData(const std::string & peer_id,
                                    const std::string & msg)
{
    LOG_F(LS_VERBOSE)<<"receive from :"<<peer_id<<" msg:"<<msg;
    Json::Reader reader;
    Json::Value jmessage;
    if (!reader.parse(msg, jmessage)) {
        LOG(WARNING) << "Received unknown message. " << msg;
        return;
    }
    std::string type;
    GetStringFromJsonObject(jmessage, "type", &type);
    if(type.compare("p2p") == 0){
        ScopedTunnel aTunnel = this->GetOrCreateTunnel(peer_id);
        if(aTunnel == NULL){
            LOG(WARNING)<<"read  p2p msg and create tunnel error "<<peer_id;
            return;
        }
        std::string peerMsg;
        GetStringFromJsonObject(jmessage, "msg", &peerMsg);
        aTunnel->OnMessageFromPeer(peer_id,peerMsg);
    }
    else if(type.compare("tunnel") == 0){
        //dispath tunnel message
        //LOG(INFO)<<"PeerTerminal::OnRouterReadData---receive tunnel msg : "<<msg;
        talk_base::Buffer buffer(msg.c_str(),msg.length());
        SignalRouterMessage(peer_id,buffer);
    }else{
        LOG(WARNING)<<"receive unexpected message from "<<peer_id;
    }
}

void PeerTerminal::OnTunnelNeedSend(const std::string &peer_id,
                                    const std::string &msg)
{
    LOG(INFO)<<"PeerTerminal::OnTunnelNeedSend----"<<peer_id.c_str();
    Json::StyledWriter writer;
    Json::Value jmessage;
    jmessage["type"] = "p2p";
    jmessage["msg"] = msg;
    std::string data = writer.write(jmessage);
    this->SendByRouter(peer_id,data);
}

ScopedTunnel PeerTerminal::GetTunnel(const std::string &peer_id)
{
    talk_base::CritScope cs(&crit_);
    std::deque<ScopedTunnel>::iterator it = tunnels_.begin();
    for (; it != tunnels_.end(); ++it) {
        if ((*it)->GetPeerID().compare(peer_id) == 0) {
            break;
        }
    }
    if (it == tunnels_.end())
        return NULL;
    return *it;
}

ScopedTunnel PeerTerminal::GetTunnel(kaerp2p::StreamProcess *stream)
{
    talk_base::CritScope cs(&crit_);
    std::deque<ScopedTunnel>::iterator it = tunnels_.begin();
    for (; it != tunnels_.end(); ++it) {
        if ((*it)->GetStreamProcess() == stream) {
            break;
        }
    }
    if (it == tunnels_.end())
        return NULL;
    return *it;
}

int PeerTerminal::CountAvailableTunnels()
{
    talk_base::CritScope cs(&crit_);
    int notUsedNum = max_tunnel_num_ - tunnels_.size();
    std::deque<ScopedTunnel>::iterator it = tunnels_.begin();
    for (; it != tunnels_.end(); ++it) {
        if ((*it)->GetPeerID().empty()) {
            notUsedNum++;
            break;
        }
    }
    return notUsedNum;
}

ScopedTunnel PeerTerminal:: GetOrCreateTunnel(const std::string &peer_id)
{
    ScopedTunnel aTunnel = this->GetTunnel(peer_id);
    if(aTunnel){
        return aTunnel;
    }
    aTunnel = this->GetTunnel("");
    if(aTunnel){
        LOG(INFO)<<"PeerTerminal::GetOrCreateTunnel---"<<"reuse a empty tunnel";
        return aTunnel;
    }
    if(tunnels_.size() < max_tunnel_num_ ||
            max_tunnel_num_ == kInfiniteTunnel) {
        aTunnel = CreateNewTunnel();
        LOG(INFO)<<"PeerTerminal::GetOrCreateTunnel---"<<
                   "create a new tunnel, total tunnel num is "<< tunnels_.size()<<
                   " and max tunnel num is "<<max_tunnel_num_;
    } else {
        LOG_F(WARNING)<<"tunnel number is more than the max number "<<tunnels_.size()<<
                        " ; max number is "<< max_tunnel_num_<<
                        " ; action is "<<tunnel_out_action;
        if(tunnel_out_action == 1){
            SendTunnelError(peer_id,"tunnel_max");
        }else if(tunnel_out_action == 2){
            aTunnel = tunnels_.front();
            std::string toRemovePeer = aTunnel->GetPeerID();
            aTunnel->DisconnectFromCurrentPeer();
            SendTunnelError(toRemovePeer,"tunnel_replace");
        }
    }
    return aTunnel;
}

ScopedTunnel PeerTerminal::CreateNewTunnel()
{
    ScopedTunnel aTunnel = new talk_base::RefCountedObject<kaerp2p::P2PConductor>();
    aTunnel->SignalNeedSendToPeer.connect(this,&PeerTerminal::OnTunnelNeedSend);
    aTunnel->SignalStreamOpened.connect(this,&PeerTerminal::OnTunnelOpened);
    aTunnel->SignalStreamClosed.connect(this,&PeerTerminal::OnTunnelClosed);
    talk_base::CritScope cs(&crit_);
    tunnels_.push_back(aTunnel);
    return aTunnel;
}

void PeerTerminal::SendTunnelError(const std::string &peer_id,
                                   const std::string errorType)
{
    Json::StyledWriter writer;
    Json::Value jmessage;
    jmessage["type"] = "tunnel";
    jmessage["command"] = "p2p_error";
    jmessage["errorType"] = errorType;
    std::string msg = writer.write(jmessage);
    this->SendByRouter(peer_id,msg);
}

LocalUdpTerminal::LocalUdpTerminal():
    localSocket(NULL),ownThread(false),isOpened(false)
{
    this->socketThread_ = new talk_base::Thread();
    this->socketThread_->Start();
    socket_factory_.reset(new talk_base::BasicPacketSocketFactory(socketThread_));
}

LocalUdpTerminal::~LocalUdpTerminal()
{
    if(localSocket){
        localSocket->Close();
        delete localSocket;
    }
    delete this->socketThread_;
}

bool LocalUdpTerminal::Initialize(const std::string &localAddr)
{
    return socketThread_->Invoke<bool>(
                talk_base::Bind(&LocalUdpTerminal::Initialize_s,this,localAddr));

}

bool LocalUdpTerminal::OpenTunnel(const std::string &peerAddr)
{
//    return socketThread_->Invoke<bool>(
//                talk_base::Bind(&LocalTerminal::OpenTunnel_s,this,peerAddr));
    if(localSocket){
        this->SignalTunnelOpened(this,peerAddr);
        this->isOpened = true;
        return true;
    }
    return false;
}

bool LocalUdpTerminal::CloseTunnel(const std::string &peerAddr)
{
    if(localSocket){
        this->SignalTunnelClosed(this,peerAddr);
        return true;
    }
    return false;
}

bool LocalUdpTerminal::SendByRouter(const std::string &peerAddr,
                                    const std::string &data)
{
    return SendByRouter(peerAddr,data.c_str(),data.size());
}

bool LocalUdpTerminal::SendByRouter(const std::string &peerAddr,
                                    const char *data, size_t len)
{
    return false;
}

bool LocalUdpTerminal::SendByTunnel(const std::string &peerAddr,
                                    const std::string &data)
{
    return SendByTunnel(peerAddr,data.c_str(),data.size());
}

bool LocalUdpTerminal::SendByTunnel(const std::string &peerAddr,
                                    const char *data, size_t len)
{
    talk_base::PacketOptions options;
    talk_base::SocketAddress remote_addr;
    remote_addr.FromString(peerAddr);
    const int onceSend = 1400;
    int leftDataLen = len;
    while(leftDataLen > 0){
        int sendLen = leftDataLen > onceSend ? onceSend:leftDataLen;
        int sent = localSocket->SendTo(data,sendLen,remote_addr,options);
        if(sent < 0){
            LOG(INFO)<<"LocalTerminal::SendByTunnel --- send "<<sendLen<<
                       " error "<<localSocket->GetError();
            return false;
        }
        leftDataLen -= sendLen;
        //socketThread_->SleepMs(1);
        //LOG_T_F(INFO)<<"send "<<sent;
    }
    return true;
}

const int kLocalSocketStartPort = 22555;
const int kLocalSocketEndPort = 22600;

bool LocalUdpTerminal::Initialize_s(const std::string &localAddr)
{
    talk_base::SocketAddress local_address(INADDR_ANY,kLocalSocketStartPort);
    if(!localAddr.empty() && !local_address.FromString(localAddr)){
        return false;
    }

    localSocket = socket_factory_->CreateUdpSocket(local_address,
                                     kLocalSocketStartPort,kLocalSocketEndPort);
    if(!localSocket){
        LOG_T_F(WARNING) << "Failed to create UDP socket bound at port";
        return false;
    }
    LOG_T_F(INFO)<<"create socket success "<<localSocket->GetLocalAddress().ToString();
    localSocket->SignalReadPacket.connect(this,&LocalUdpTerminal::OnPackage);
    localSocket->SetOption(talk_base::Socket::OPT_NODELAY,1);

    return true;
}

bool LocalUdpTerminal::SendByTunnel_s(const std::string &peerAddr, const char *data, size_t len)
{
    talk_base::PacketOptions options;
    talk_base::SocketAddress remote_addr;
    remote_addr.FromString(peerAddr);
    const int onceSend = 1400;
    int leftDataLen = len;
    while(leftDataLen > 0){
        int sendLen = leftDataLen > onceSend ? onceSend:leftDataLen;
        int sent = localSocket->SendTo(data,sendLen,remote_addr,options);
        if(sent < 0){
            LOG(INFO)<<"LocalTerminal::SendByTunnel --- send "<<sendLen<<
                       " error "<<localSocket->GetError();
            return false;
        }
        leftDataLen -= sendLen;
        //socketThread_->SleepMs(1);
        LOG_T_F(INFO)<<"send "<<sent;
    }
    return true;

}

void LocalUdpTerminal::OnPackage(talk_base::AsyncPacketSocket *socket,
                                 const char *buf, size_t size,
                                 const talk_base::SocketAddress &remote_addr,
                                 const talk_base::PacketTime &packet_time)
{

    std::string peerAddr = remote_addr.ToString();
    if(!isOpened){
        this->SignalTunnelOpened(this,peerAddr);
        isOpened = true;
    }
    talk_base::Buffer buffer(buf,size);
    if(socket == localSocket){
        this->SignalTunnelMessage(peerAddr,buffer);
    }
}

}


