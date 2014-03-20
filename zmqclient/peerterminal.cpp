#include "peerterminal.h"

PeerTerminal::PeerTerminal()
{
}

int PeerTerminal::Initialize(const std::string &router, const std::string &id)
{
    client_.reset(new PeerConnectionClientDealer());
    int ret = client_->Connect(router,id);

    //a new conductor is created at here
    conductor_ = new talk_base::RefCountedObject<kaerp2p::P2PConductor>(client_.get());
    conductor_->SignalStreamOpened.connect(this,&PeerTerminal::OnTunnelOpened);
    this->tunnel_stream_ = NULL;
    return ret;
}


void PeerTerminal::ConnectToPeer(const std::string & peer_id)
{
    conductor_->ConnectToPeer(peer_id);


}

void PeerTerminal::CloseTunnel()
{
    conductor_->DisconnectFromCurrentPeer();
    this->tunnel_stream_ = NULL;
}

bool PeerTerminal::TunnelOpened()
{
    return (this->tunnel_stream_ != NULL);
}

int PeerTerminal::SendByRouter(const std::string &peer_id, const std::string &data)
{
    client_->SendToPeer(peer_id,data);
    return 0;
}

int PeerTerminal::SendByTunnel(const std::string &data)
{
    ASSERT(tunnel_stream_);
    tunnel_stream_->WriteStream(data.c_str(),data.length());
    return 0;
}

int PeerTerminal::SendByTunnel(const char *data, size_t len)
{
    ASSERT(tunnel_stream_);
    tunnel_stream_->WriteStream(data,len);
    return 0;
}

int PeerTerminal::AskTunnelVideo()
{
    talk_base::Buffer msgSend;
    int msgLen = sizeof(KEVideoServerReq);
    msgSend.SetLength(msgLen);
    KEVideoServerReq * pReqMsg;
    pReqMsg = (KEVideoServerReq *)msgSend.data();
    pReqMsg->protocal = PROTOCOL_HEAD;
    pReqMsg->msgType = KEMSG_TYPE_VIDEOSERVER;
    pReqMsg->msgLength = msgLen;
    pReqMsg->clientID = 0;
    pReqMsg->channelNo = 1;
    pReqMsg->videoID = 0;
    pReqMsg->video = 0;
    pReqMsg->listen = 0;
    pReqMsg->talk = 0;
    pReqMsg->protocalType = 0;
    pReqMsg->transSvrIp = 0;
    return this->SendByTunnel(msgSend.data(),msgSend.length());

}


void PeerTerminal::OnTunnelOpened(kaerp2p::StreamProcess *tunnel)
{
    ASSERT(tunnel == conductor_->GetStreamProcess());
    LOG(INFO)<< __FUNCTION__;
    this->tunnel_stream_ = tunnel;
    tunnel_stream_->SignalReadData.connect(this,&PeerTerminal::OnTunnelReadData);

    this->SignalTunnelOpened(conductor_->GetPeerID());
}

void PeerTerminal::OnTunnelReadData(kaerp2p::StreamProcess *tunnel, size_t len)
{
    ASSERT(tunnel == conductor_->GetStreamProcess());
    //LOG(INFO)<< __FUNCTION__ << " read " << len;
    char  * buffer = new char[len];
    size_t readLen;
    bool result = tunnel->ReadStream(buffer,len,&readLen);
    if(!result){
        LOG(WARNING)<<__FUNCTION__<<"--read stream error";
        return ;
    }

//    this->ExtractMessage(buffer,readLen);


    talk_base::Buffer data(buffer,readLen);
    delete buffer;
    this->SignalTunnelMessage(conductor_->GetPeerID(),data);
}

bool PeerTerminal::ExtractMessage(const char *data, size_t len)
{
    int nRead= 0;
    talk_base::MemoryStream buffer(data,len);
    const int headLen = 10;
    talk_base::StreamResult result;
    size_t read_bytes;
    int error;
    static int bufPos = 0;          //bufPos is msgRecv's write position
    static int toRead = 0;
    static const int msgMaxLen = 8192;

    while(true)
    {
        if (msgRecv.length() == 0)//上一个消息已经读取完成
        {
            result = buffer.Read(headBuf+bufPos,headLen-bufPos,&read_bytes,&error);
            if(result == talk_base::SR_EOS){
                break;
            }
            if(read_bytes < headLen-bufPos)//消息头在最后几个字节，记录读取的字节，下次继续读取。
            {
                LOG(INFO)<<"Continue Read head in new package\r\n ";
                bufPos = read_bytes;
                break;
            }
            unsigned char  protocal = headBuf[0];
            int msgLen = *((int*)&headBuf[2]);
            if (protocal != PROTOCOL_HEAD||  msgLen>msgMaxLen)
            {
                LOG(WARNING)<<"The message Protocal Head error, Clear the recv buffer!\r\n";
                msgRecv.SetLength(0);
                break;
            }
            msgRecv.SetLength(msgLen);
            bufPos = 0;
            memcpy(msgRecv.data(),headBuf,headLen);
            bufPos += headLen;
            toRead =  msgLen-headLen;
            if (toRead != 0)//防止 headLen 越界
            {
                result = buffer.Read(msgRecv.data()+bufPos,toRead,&read_bytes,&error);
                if(result == talk_base::SR_EOS){
                    break;
                }
                bufPos += read_bytes;
                toRead -= read_bytes;
            }
        }
        else//上一个消息未完成读取
        {
            result = buffer.Read(msgRecv.data()+bufPos,toRead,&read_bytes,&error);
            if(result == talk_base::SR_EOS){
                break;
            }
            if (read_bytes < toRead){
                LOG(INFO)<<"to read more and more!";
            }
            bufPos += read_bytes;
            toRead -= read_bytes;
        }
        if(toRead == 0 && bufPos == msgRecv.length())//全部读取
        {
            this->OnMessageRespond(msgRecv);
            msgRecv.SetLength(0);
            bufPos = 0;
        }
    }
    return true;
}

void PeerTerminal::OnMessageRespond(talk_base::Buffer &msg)
{

}




