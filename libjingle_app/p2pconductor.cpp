#include "p2pconductor.h"

#include "talk/base/json.h"
#include "talk/base/bind.h"
#include "defaults.h"
namespace  kaerp2p {

const int kConnectTimeout = 13000; // close DeletePeerConnection after 13s without connect
const int kDisConnectTimeout = 2000;

// Names used for a IceCandidate JSON object.
const char kCandidateSdpMidName[] = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[] = "candidate";

// Names used for a SessionDescription JSON object.
const char kSessionDescriptionTypeName[] = "type";
const char kSessionDescriptionSdpName[] = "sdp";

const char kByeMessage[] = "BYE";
const char kEndMsg[] = "SEEYOU";

class DummySetSessionDescriptionObserver
        : public webrtc::SetSessionDescriptionObserver {
public:
    static DummySetSessionDescriptionObserver* Create() {
        return new talk_base::RefCountedObject<DummySetSessionDescriptionObserver>();
    }
    virtual void OnSuccess() {
        LOG(INFO) << __FUNCTION__;
    }
    virtual void OnFailure(const std::string& error) {
        LOG(INFO) << __FUNCTION__ << " " << error;
    }

protected:
    DummySetSessionDescriptionObserver() {}
    ~DummySetSessionDescriptionObserver() {}
};

P2PConductor::P2PConductor():
    stream_thread_(NULL),signal_thread_(NULL),tunnelState(kTunnelNew)
{
    if(stream_thread_ == NULL){
        stream_thread_ = new talk_base::Thread();
        stream_thread_->Start();
    }
    if(signal_thread_ == NULL){
        signal_thread_ = new talk_base::Thread();
        signal_thread_->Start();
    }
}

P2PConductor::~P2PConductor()
{
    signal_thread_->Invoke<void>(talk_base::Bind(
        &P2PConductor::DeletePeerConnection,this));
    if(signal_thread_)
        delete signal_thread_;
    if(stream_thread_)
        delete stream_thread_;
}


bool P2PConductor::ConnectToPeer(const std::string &peer_id)
{
    if(this->tunnelState == kTunnelDisconnecting){
        LOG(INFO)<<"the tunnel is disconnecting,should not connect!";
        return false;
    }
    if (peer_connection_.get()) {
        LOG(LS_INFO) <<"peer connection already connect";
        return true;
    }
    if (InitializePeerConnection()) {
        peer_id_ = peer_id;
        peer_connection_->CreateOffer(this,"tcp");
    } else {
        LOG(LS_INFO) << "initialize connection error";
        return false;
    }
    return true;
}

void P2PConductor::DisconnectFromCurrentPeer()
{
    if(this->tunnelState == kTunnelDisconnecting){
        LOG(INFO)<<"the tunnel is disconnecting!";
        return;
    }
    if ( peer_connection_.get() ) {
        signal_thread_->Post(this,MSG_DISCONNECT);
        SignalNeedSendToPeer(peer_id_,kByeMessage);
        this->signal_thread_->PostDelayed(kDisConnectTimeout,this,
                                          MSG_DISCONNECT_TIMEOUT);
    }
}


StreamProcess *P2PConductor::GetStreamProcess()
{
    return stream_process_.get();
}

void P2PConductor::OnTunnelEstablished()
{
    ASSERT(stream_process_);
    LOG_T_F(INFO)<<" established";
    signal_thread_->Clear(this,MSG_CONNECT_TIMEOUT);
    this->setTunnelState(kTunnelEstablished);
    SignalStreamOpened(this->GetPeerID());
}

void P2PConductor::OnTunnelTerminate(StreamProcess * stream)
{
    //ASSERT(this->GetStreamProcess() == stream);
    LOG(INFO) << "P2PConductor::OnTunnelTerminate---no nothing";
}

void P2PConductor::OnMessageFromPeer_s(const std::string &peerId,
                                       const std::string &message)
{
    //为了防止在断开链接是依然有消息从对端发送过来(对端尚未收到bye消息时正在获取candidate),
    //在收到bye消息后发送seeyou消息,用于对tunnel的状态进行控制
    //发送bye消息后进入disconnecting状态,
    if (tunnelState == kTunnelDisconnecting) {
        if( message.length() == ( sizeof(kEndMsg) - 1 ) &&
                message.compare(kEndMsg) == 0 ) {
            LOG(INFO)<<"receive end message from "<<peerId <<",tunnel is end";
            ConductorClose();
        } else {
            LOG(WARNING)<<"P2PConductor::OnMessageFromPeer---"<<
                          "should not receive message when tunnel is closing";
        }
        return;
    }

    if ( message.length() == (sizeof(kByeMessage) - 1 ) &&
            message.compare(kByeMessage) == 0) {
        LOG(INFO)<<"receive bye message from "<<peerId;
        if (peerId == peer_id_ && peer_connection_.get()) {
            SignalNeedSendToPeer(peer_id_,kEndMsg);
            DeletePeerConnection();
            ConductorClose();
        }
        return;
    }

    if(!peer_connection_.get()) { //kTunnelClosed状态
        if(!this->peer_id_.empty()){
            LOG(LS_ERROR) << "peer_id_ is not empty when peer_connection_ is not set";
            return;
        }
        peer_id_ = peerId;
        if (!InitializePeerConnection()) {
            LOG(LS_ERROR) << "Failed to initialize our PeerConnection instance";
            return;
        }
    } else if (peerId != peer_id_) {
        ASSERT(!peer_id_.empty());
        LOG(WARNING) << "Received a message from unknown peer while already in"
                        " a conversation with a different peer.";
        return;
    }

    Json::Reader reader;
    Json::Value jmessage;
    if (!reader.parse(message, jmessage)) {
        LOG(WARNING) << "Received unknown message. " << message;
        return;
    }
    std::string type;

    GetStringFromJsonObject(jmessage, kSessionDescriptionTypeName, &type);
    if (!type.empty()) {
        std::string sdp;
        if (!GetStringFromJsonObject(
                    jmessage, kSessionDescriptionSdpName, &sdp)) {
            LOG(WARNING) << "Can't parse received session description message.";
            return;
        }
        webrtc::SessionDescriptionInterface* session_description(
                    webrtc::CreateSessionDescription(type, sdp));
        if (!session_description) {
            LOG(WARNING) << "Can't parse received session description message.";
            return;
        }
        LOG(INFO) << " Received session description :" << message;
        peer_connection_->SetRemoteDescription(
                    DummySetSessionDescriptionObserver::Create(),
                    session_description);
        if (session_description->type() ==
                webrtc::SessionDescriptionInterface::kOffer) {
            peer_connection_->CreateAnswer(this);
        }
        return;
    } else {
        std::string sdp_mid;
        int sdp_mlineindex = 0;
        std::string sdp;
        if (!GetStringFromJsonObject(jmessage, kCandidateSdpMidName, &sdp_mid) ||
                !GetIntFromJsonObject(jmessage, kCandidateSdpMlineIndexName,
                                      &sdp_mlineindex) ||
                !GetStringFromJsonObject(jmessage, kCandidateSdpName, &sdp)) {
            LOG(WARNING) << "Can't parse received message.";
            return;
        }
        talk_base::scoped_ptr<webrtc::IceCandidateInterface> candidate(
                    webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp));
        if (!candidate.get()) {
            LOG(WARNING) << "Can't parse received candidate message.";
            return;
        }
        LOG(INFO) << " Received candidate :" << message;
        if (!peer_connection_->AddIceCandidate(candidate.get())) {
            LOG(WARNING) << "Failed to apply the received candidate";
            return;
        }
        return;
    }
}

void P2PConductor::ConductorClose()
{
    signal_thread_->Clear(this,MSG_DISCONNECT_TIMEOUT);
    setTunnelState(kTunnelClosed);
    SignalStreamClosed(peer_id_);
    peer_id_.clear();
    stream_process_.reset();
}

void P2PConductor::setTunnelState(P2PConductor::TunnelState state)
{
    tunnelState = state;
}

void P2PConductor::AddIceServer(const std::string &uri,
                                const std::string &username,
                                const std::string &password)
{
    PeerTunnelInterface::IceServer server;
    server.uri = uri;
    server.username = username.empty()? "lht":username;
    server.password = password.empty()? "123456":password;
    g_servers.push_back(server);
}
//将g_servers置为jstrServers中的server
void P2PConductor::AddIceServers(std::string jstrServers)
{

    Json::Reader reader;
    Json::Value jservers;
    if (!reader.parse(jstrServers, jservers)) {
        LOG(WARNING) << "P2PConductor::AddIceServers---"<<
                        "unknown json string " << jstrServers;
        return;
    }
    //每次都重新添加
    g_servers.clear();
    std::vector<Json::Value> jServersArray;
    LOG_F(INFO)<<" add ice servers "<< jstrServers;
    if(JsonArrayToValueVector(jservers,&jServersArray)){
        for(int i=0;i<jServersArray.size();i++){
            Json::Value jserver = jServersArray[i];
            std::string uri,username,password;
            GetStringFromJsonObject(jserver,"uri",&uri);
            GetStringFromJsonObject(jserver,"username",&username);
            GetStringFromJsonObject(jserver,"password",&password);
            P2PConductor::AddIceServer(uri,username,password);
        }
    }
}


bool P2PConductor::InitializePeerConnection()
{
    if(g_servers.empty()){
        //lht TODO: turn server should get by the server
//        P2PConductor::AddIceServer("stun:222.174.213.181:5389","","");
        P2PConductor::AddIceServer(GetPeerConnectionString(),"","");
//        P2PConductor::AddIceServer("turn:222.174.213.185:5766",
//                                   "lht","123456");
    }

    stream_process_.reset(new StreamProcess(stream_thread_));
    stream_process_->SignalOpened.connect(
                this,&P2PConductor::OnTunnelEstablished);
    stream_process_->SignalClosed.connect(
                this,&P2PConductor::OnTunnelTerminate);

    talk_base::scoped_refptr<PeerTunnel> pt (
                new talk_base::RefCountedObject<PeerTunnel>(
                    signal_thread_,stream_thread_));
    if(!pt->Initialize(g_servers,this)){
        return false;
    }
    setTunnelState(kTunnelConnecting);

    peer_connection_ = PeerTunnelProxy::Create(pt->signaling_thread(), pt);
    LOG(INFO)<<"P2PConductor::InitializePeerConnection---connect timeout is "<<kConnectTimeout;
    signal_thread_->PostDelayed(kConnectTimeout,this,MSG_CONNECT_TIMEOUT);

    return true;
}

void P2PConductor::DeletePeerConnection()
{
    setTunnelState(kTunnelDisconnecting);
    signal_thread_->Clear(this,MSG_CONNECT_TIMEOUT);
    //when close peer_connection the session will terminate and destroy the channels
    //the channel destroy will make the StreamProcess clean up
    if(peer_connection_.get()){
        peer_connection_->Close();
        peer_connection_.release();
    }
}

void P2PConductor::OnSuccess(SessionDescriptionInterface *desc)
{
    peer_connection_->SetLocalDescription(
                DummySetSessionDescriptionObserver::Create(),desc);
    //get stream process
    talk_base::StreamInterface* stream = peer_connection_->GetStream();
    if(!stream_process_)
    {
        LOG_F(WARNING) << " get stream error";
        return ;
    }
    bool result = stream_thread_->Invoke<bool>(
                talk_base::Bind(&StreamProcess::ProcessStream,
                                this->GetStreamProcess(),stream));
    if(!result){
        LOG(WARNING)<<"stream process faild";
        return;
    }
    Json::StyledWriter writer;
    Json::Value jmessage;
    jmessage[kSessionDescriptionTypeName] = desc->type();
    std::string sdp;
    desc->ToString(&sdp);
    jmessage[kSessionDescriptionSdpName] = sdp;
    std::string msg = writer.write(jmessage);
    LOG(INFO) << "P2PConductor::OnSuccess---" << "session sdp is " << msg;
    SignalNeedSendToPeer(peer_id_,msg);
}

void P2PConductor::OnFailure(const std::string &error)
{
    LOG(LERROR) <<"P2PConductor::OnFailure"<< error;
}

void P2PConductor::OnError()
{
    LOG(LS_ERROR) << "P2PConductor::OnError---" << "offer error";
    DisconnectFromCurrentPeer();
}

void P2PConductor::OnRenegotiationNeeded()
{
    LOG(LS_ERROR) << "P2PConductor::OnRenegotiationNeeded";
}

void P2PConductor::OnIceCandidate(const IceCandidateInterface *candidate)
{
    Json::StyledWriter writer;
    Json::Value jmessage;
    jmessage[kCandidateSdpMidName] = candidate->sdp_mid();
    jmessage[kCandidateSdpMlineIndexName] = candidate->sdp_mline_index();
    std::string sdp;
    if (!candidate->ToString(&sdp)) {
        LOG(LS_ERROR) << "Failed to serialize candidate";
        return;
    }
    jmessage[kCandidateSdpName] = sdp;
    std::string msg = writer.write(jmessage);
    LOG(INFO) << "P2PConductor::OnIceCandidate---" << msg;
    SignalNeedSendToPeer(peer_id_,msg);
}

void P2PConductor::OnIceGatheringChange(IceObserver::IceGatheringState new_state)
{
    LOG(INFO) << "P2PConductor::OnIceGatheringChange---" <<new_state;
}

struct PeerMessageParams : public talk_base::MessageData {
    PeerMessageParams(std::string id,std::string msg):peerId(id),message(msg) {}
    ~PeerMessageParams() {}
    std::string peerId;
    std::string message;
};

void P2PConductor::OnMessage(talk_base::Message *msg)
{
    if(msg->message_id == MSG_CONNECT_TIMEOUT){
        LOG(INFO)<<"P2PConductor::OnMessage-----"<<"connect is timeout";
        DisconnectFromCurrentPeer();
    }else if(msg->message_id == MSG_PEER_MESSAGE){
        PeerMessageParams * param = static_cast<PeerMessageParams*>(msg->pdata);
        this->OnMessageFromPeer_s(param->peerId,param->message);
        delete param;
    }else if(msg->message_id == MSG_DISCONNECT){

        DeletePeerConnection();
    }else if(msg->message_id == MSG_DISCONNECT_TIMEOUT){
        LOG(WARNING)<<"P2PConductor::OnMessage-----"<<"disconnect timeout";
        ConductorClose();
    }
}



void P2PConductor::OnMessageFromPeer(const std::string &peer_id,
                                     const std::string &message)
{
    ASSERT(!message.empty());
    if(peer_id != peer_id_ && !peer_id_.empty()){
        LOG(WARNING)<<"P2PConductor::OnMessageFromPeer---peer id is wrong";
        return;
    }
    //LOG(INFO)<<"P2PConductor::OnMessageFromPeer---from - "<<peer_id<<", msg -"<<message;
    //为了防止tunnelState的冲突，我们将tunnelstate的操作放在signal_thread中.
    PeerMessageParams * param = new PeerMessageParams(peer_id,message);
    signal_thread_->Send(this,MSG_PEER_MESSAGE,param);
    //    signal_thread_->Invoke<void>(
    //                talk_base::Bind(&P2PConductor::OnMessageFromPeer_s,
    //                                this,peer_id,message));
    return;
}

PeerTunnelInterface::IceServers P2PConductor::g_servers;

}
