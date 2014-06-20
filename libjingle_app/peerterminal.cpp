#include "peerterminal.h"
#include "KeMessage.h"
#include "talk/base/json.h"

namespace kaerp2p {



  PeerTerminal::PeerTerminal():
    client_(0)
  {

  }

  int PeerTerminal::Initialize(kaerp2p::PeerConnectionClientInterface * client,
                               int max_tunnel)
  {
    max_tunnel_num_ = max_tunnel;
    this->client_ = client;
    //int ret = client_->Connect(router,id);
    client_->SignalMessageFromPeer.connect(this,&PeerTerminal::OnRouterReadData);
    return 0;
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
    //stream->SignalClosed.connect(this,&PeerTerminal::OnTunnelClosed);
    this->SignalTunnelOpened(this,aTunnel->GetPeerID());
  }

  void PeerTerminal::OnTunnelClosed(kaerp2p::StreamProcess *stream)
  {
    LOG(INFO)<< __FUNCTION__;
    ScopedTunnel aTunnel = this->GetTunnel(stream);
    if(aTunnel == NULL){
        LOG(WARNING)<<"PeerTerminal::OnTunnelClosed---cannot get tunnel by stream";
        return ;
      }

    this->SignalTunnelClosed(this,aTunnel->GetPeerID());
  }

  void PeerTerminal::OnTunnelReadData(kaerp2p::StreamProcess *stream, size_t len)
  {
    //ASSERT(tunnel == conductor_->GetStreamProcess());
    //LOG(INFO)<< __FUNCTION__ << " read " << len;
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
    else if(type.compare("tunnel") == 0){
        //dispath tunnel message
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
      }
    aTunnel = this->GetTunnel("");
    if(aTunnel){
        LOG(INFO)<<"PeerTerminal::GetOrCreateTunnel---"<<
                   "reuse a empty tunnel, total is "<<tunnels_.size();
        return aTunnel;
      }
    if(tunnels_.size() <= max_tunnel_num_ ||
       max_tunnel_num_ == kInfiniteTunnel){
        aTunnel = new talk_base::RefCountedObject<kaerp2p::P2PConductor>();
        aTunnel->SignalNeedSendToPeer.connect(this,&PeerTerminal::OnTunnelNeedSend);
        aTunnel->SignalStreamOpened.connect(this,&PeerTerminal::OnTunnelOpened);
        aTunnel->SignalStreamClosed.connect(this,&PeerTerminal::OnTunnelClosed);
        tunnels_.push_back(aTunnel);
        LOG(INFO)<<"PeerTerminal::GetOrCreateTunnel---"<<
                   "create a new tunnel, total tunnel num is "<< tunnels_.size();
      } else {
        LOG(WARNING) <<"PeerTerminal::GetOrCreateTunnel---"<<
                       "tunnel number is more than the max number "<<
                       tunnels_.size();
      }
    return aTunnel;
  }

}


