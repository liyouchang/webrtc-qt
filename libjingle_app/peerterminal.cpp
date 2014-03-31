#include "peerterminal.h"
#include "KeMessage.h"
#include "talk/base/json.h"

PeerTerminal::PeerTerminal():
    client_(0)
{

}

int PeerTerminal::Initialize(kaerp2p::PeerConnectionClientInterface * client)
{
    max_tunnel_num_ = 4;
    this->client_ = client;
    //int ret = client_->Connect(router,id);
    client_->SignalMessageFromPeer.connect(this,&PeerTerminal::OnRouterReadData);
    return 0;
}


int PeerTerminal::ConnectToPeer(const std::string & peer_id)
{
    ScopedTunnel aTunnel = this->GetOrCreateTunnel(peer_id);
    if(aTunnel== NULL){
        LOG(WARNING)<< "not avaliable tunnel";
        return -1;
    }
    bool ret = aTunnel->ConnectToPeer(peer_id);
    return ret;
}

int PeerTerminal::CloseTunnel(const std::string &peer_id)
{
    ScopedTunnel aTunnel = this->GetTunnel(peer_id);
    if(aTunnel == NULL){
        return -1;
    }
    aTunnel->DisconnectFromCurrentPeer();
    return 0;
}


int PeerTerminal::SendByRouter(const std::string &peer_id, const std::string &data)
{
    client_->SendToPeer(peer_id,data);
    return 0;
}

int PeerTerminal::SendByTunnel(const std::string &peer_id,const std::string &data)
{
//    ASSERT(tunnel_stream_);
    ScopedTunnel aTunnel = this->GetTunnel(peer_id);
    if(aTunnel == NULL){
        return -1;
    }
    aTunnel->GetStreamProcess()->WriteStream(data.c_str(),data.length());
    return 0;
}

int PeerTerminal::SendByTunnel(const std::string &peer_id,
                               const char *data, size_t len)
{
    ScopedTunnel aTunnel = this->GetTunnel(peer_id);
    if(aTunnel == NULL){

        return -1;
    }
    aTunnel->GetStreamProcess()->WriteStream(data,len);
    return 0;
}


void PeerTerminal::OnTunnelOpened(kaerp2p::StreamProcess *stream)
{
    //ASSERT(tunnel == conductor_->GetStreamProcess());
    LOG(INFO)<< __FUNCTION__;
    ScopedTunnel aTunnel = this->GetTunnel(stream);
    if(aTunnel == NULL){
        LOG(WARNING)<<"cannot get tunnel by stream";
        return ;
    }
    //this->tunnel_stream_ = tunnel;
    stream->SignalReadData.connect(this,&PeerTerminal::OnTunnelReadData);
    stream->SignalClosed.connect(this,&PeerTerminal::OnTunnelClosed);
    this->SignalTunnelOpened(this,aTunnel->GetPeerID());
}

void PeerTerminal::OnTunnelClosed(kaerp2p::StreamProcess *stream)
{
    LOG(INFO)<< __FUNCTION__;
    ScopedTunnel aTunnel = this->GetTunnel(stream);
    if(aTunnel == NULL){
        LOG(WARNING)<<"cannot get tunnel by stream";
        return ;
    }

    this->SignalTunnelClosed(this,aTunnel->GetPeerID());
}

void PeerTerminal::OnTunnelReadData(kaerp2p::StreamProcess *stream, size_t len)
{
    //ASSERT(tunnel == conductor_->GetStreamProcess());
    LOG(INFO)<< __FUNCTION__ << " read " << len;
    ScopedTunnel aTunnel = this->GetTunnel(stream);
    if(aTunnel == NULL){
        LOG(WARNING)<<"cannot get tunnel by stream";
        return ;
    }

    char  * buffer = new char[len];
    size_t readLen;
    bool result = stream->ReadStream(buffer,len,&readLen);
    if(!result){
        LOG(WARNING)<<__FUNCTION__<<"--read stream error";
        return ;
    }
    talk_base::Buffer data(buffer,readLen);
    delete buffer;
    this->SignalTunnelMessage(aTunnel->GetPeerID(),data);
}

void PeerTerminal::OnRouterReadData(const std::string & peer_id, const std::string & msg)
{
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
            LOG(WARNING)<<"read  p2p msg from unknown peer "<<peer_id;
            return;
        }
        std::string peerMsg;
        GetStringFromJsonObject(jmessage, "msg", &peerMsg);
        aTunnel->OnMessageFromPeer(peer_id,peerMsg);
    }
    else{
        //TODO: other message dispatch
        SignalRouterMessage(peer_id,msg);
    }

}

void PeerTerminal::OnTunnelNeedSend(const std::string &peer_id, const std::string &msg)
{
    Json::StyledWriter writer;
    Json::Value jmessage;
    jmessage["type"] = "p2p";
    jmessage["msg"] = msg;
    std::string data = writer.write(jmessage);
    this->SendByRouter(peer_id,data);

}

ScopedTunnel PeerTerminal::GetTunnel(const std::string &peer_id)
{
    std::vector<ScopedTunnel>::iterator it = tunnels_.begin();
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
    std::vector<ScopedTunnel>::iterator it = tunnels_.begin();
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
    int notUsedNum = max_tunnel_num_ - tunnels_.size();
    std::vector<ScopedTunnel>::iterator it = tunnels_.begin();
    for (; it != tunnels_.end(); ++it) {
      if ((*it)->GetPeerID().empty()) {
        notUsedNum++;
        break;
      }
    }

    return notUsedNum;

}

ScopedTunnel PeerTerminal::GetOrCreateTunnel(const std::string &peer_id)
{
    ScopedTunnel aTunnel = this->GetTunnel(peer_id);
    if(aTunnel){
        return aTunnel;
    }else if(tunnels_.size() <= max_tunnel_num_){
        aTunnel = new talk_base::RefCountedObject<kaerp2p::P2PConductor>();
        aTunnel->SignalNeedSendToPeer.connect(this,&PeerTerminal::OnTunnelNeedSend);
        aTunnel->SignalStreamOpened.connect(this,&PeerTerminal::OnTunnelOpened);
        tunnels_.push_back(aTunnel);
    }
    else{
        aTunnel = this->GetTunnel("");
    }
    return aTunnel;
}



