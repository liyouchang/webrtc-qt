
#include "ServerConductor.h"
#include "defaults.h"
#include "talk/p2p/client/basicportallocator.h"
#include "talk/base/json.h"
#include "talk/base/bind.h"
namespace kaerp2p {

// Names used for a IceCandidate JSON object.
const char kCandidateSdpMidName[] = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[] = "candidate";

// Names used for a SessionDescription JSON object.
const char kSessionDescriptionTypeName[] = "type";
const char kSessionDescriptionSdpName[] = "sdp";



talk_base::SocketAddress stun_addr("stun.l.google.com", 19302);




class DummySetSessionDescriptionObserver
        : public webrtc::SetSessionDescriptionObserver {
public:
    static DummySetSessionDescriptionObserver* Create() {
        return
                new talk_base::RefCountedObject<DummySetSessionDescriptionObserver>();
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

ServerConductor::ServerConductor(PeerConnectionClient *client):
    peer_id_(-1),
    client_(client),
    streamprocess_(NULL)
{
    client_->RegisterObserver(this);
    //stream_thread_ = talk_base::Thread::Current();
    client_thread_ = talk_base::Thread::Current();
    //    allocator_ = new cricket::BasicPortAllocator(
    //                &network_manager_, stun_addr, talk_base::SocketAddress(),
    //                talk_base::SocketAddress(), talk_base::SocketAddress());
    //    allocator_.reset(new cricket::BasicPortAllocator(
    //                         &network_manager_, stun_addr, talk_base::SocketAddress(),
    //                         talk_base::SocketAddress(), talk_base::SocketAddress()));

    //    allocator_->set_flags(cricket::PORTALLOCATOR_ENABLE_BUNDLE |
    //                          cricket::PORTALLOCATOR_ENABLE_SHARED_UFRAG |
    //                          cricket::PORTALLOCATOR_ENABLE_SHARED_SOCKET);


}

ServerConductor::~ServerConductor()
{
}

void ServerConductor::StartLogin(const std::string &server, int port)
{
    if (client_->is_connected())
        return;
    server_ = server;
    client_->Connect(server, port, GetPeerName());
}

void ServerConductor::DisconnectFromServer()
{
    if (client_->is_connected())
        client_->SignOut();
}

void ServerConductor::ConnectToPeer(int peer_id)
{
    ASSERT(peer_id_ == -1);
    ASSERT(peer_id != -1);

    if (peer_connection_.get()) {
        //        main_wnd_->ShowMessageBox("Error",
        //                                  "We only support connecting to one peer at a time", true);
        LOG(LS_INFO) <<"error";
        return;
    }

    if (InitializePeerConnection()) {
        peer_id_ = peer_id;
        peer_connection_->CreateOffer(this);
    } else {
        LOG(LS_INFO) <<"error";
        //main_wnd_->ShowMessageBox("Error", "Failed to initialize PeerConnection", true);
    }
}

void ServerConductor::DisconnectFromCurrentPeer()
{
    LOG(INFO) << __FUNCTION__;
    if (peer_connection_.get()) {
        client_->SendHangUp(peer_id_);
        DeletePeerConnection();
    }

    //    if (main_wnd_->IsWindow())
    //        main_wnd_->SwitchToPeerList(client_->peers());
}

StreamProcess *ServerConductor::GetStreamProcess()
{
    return streamprocess_;
}

void ServerConductor::UIThreadCallback(int msg_id, void *data)
{
    switch (msg_id) {
    case PEER_CONNECTION_CLOSED:
        LOG(INFO) << "PEER_CONNECTION_CLOSED";
        DeletePeerConnection();

        //ASSERT(active_streams_.empty());
        //        if (main_wnd_->IsWindow()) {
        //            if (client_->is_connected()) {
        //                main_wnd_->SwitchToPeerList(client_->peers());
        //            } else {
        //                main_wnd_->SwitchToConnectUI();
        //            }
        //        } else {
        //            DisconnectFromServer();
        //        }
        break;

    case SEND_MESSAGE_TO_PEER: {
        LOG(INFO) << "SEND_MESSAGE_TO_PEER";

        talk_base::TypedMessageData<std::string *> * msgData = NULL;

        std::string* msg = reinterpret_cast<std::string*>(data);
        if (msg) {
            // For convenience, we always run the message through the queue.
            // This way we can be sure that messages are sent to the server
            // in the same order they were signaled without much hassle.
            //pending_messages_.push_back(msg);
            msgData = new talk_base::TypedMessageData<std::string *>(msg);
        }
        //stream_thread_->Post(this,SEND_MESSAGE_TO_PEER,msgData);
        client_thread_->Post(this,SEND_MESSAGE_TO_PEER,msgData);
//        if (!pending_messages_.empty() && !client_->IsSendingMessage()) {
//            msg = pending_messages_.front();
//            pending_messages_.pop_front();

//            if (!client_->SendToPeer(peer_id_, *msg) && peer_id_ != -1) {
//                LOG(LS_ERROR) << "SendToPeer failed";
//                DisconnectFromServer();
//            }
//            std::cout<<"send msg : "<< *msg <<std::endl;
//            delete msg;
//        }

//        if (!peer_connection_.get())
//            peer_id_ = -1;

        break;
    }

    case PEER_CONNECTION_ERROR:
        LOG(INFO)<<"Error an unknown error occurred";
        break;

    case NEW_STREAM_ADDED: {
        break;
    }

    case STREAM_REMOVED: {
        break;
    }

    default:
        ASSERT(false);
        break;
    }
}

void ServerConductor::OnError()
{
    LOG(LS_ERROR) << __FUNCTION__;
    this->UIThreadCallback(PEER_CONNECTION_ERROR, NULL);

}



bool ServerConductor::InitializePeerConnection()
{
    PeerTunnelInterface::IceServers servers;
    PeerTunnelInterface::IceServer server;
    server.uri = GetPeerConnectionString();
    servers.push_back(server);

    stream_thread_ =  new talk_base::Thread();
    stream_thread_->Start();

    talk_base::scoped_refptr<PeerTunnel> pt (
                new talk_base::RefCountedObject<PeerTunnel>(client_thread_,stream_thread_));
    if(!pt->Initialize(servers,this)){
        return false;
    }

    peer_connection_ = PeerTunnelProxy::Create(pt->signaling_thread(), pt);

    return true;
}

void ServerConductor::DeletePeerConnection()
{
    peer_connection_.release();
    //active_streams_.clear();
    //main_wnd_->StopLocalRenderer();
    //main_wnd_->StopRemoteRenderer();
    //peer_connection_factory_ = NULL;
    peer_id_ = -1;
}

void ServerConductor::OnSuccess(webrtc::SessionDescriptionInterface* desc)
{
    LOG(INFO) << __FUNCTION__;

    peer_connection_->SetLocalDescription(DummySetSessionDescriptionObserver::Create(),desc);
    Json::StyledWriter writer;
    Json::Value jmessage;
    jmessage[kSessionDescriptionTypeName] = desc->type();
    std::string sdp;
    desc->ToString(&sdp);
    jmessage[kSessionDescriptionSdpName] = sdp;

        talk_base::StreamInterface* stream = peer_connection_->GetStream();
        if(!streamprocess_)
            streamprocess_ = new StreamProcess(stream_thread_);

        bool result = stream_thread_->Invoke<bool>(
                    talk_base::Bind(&StreamProcess::ProcessStream,streamprocess_,stream));

        if(!result){
            LOG(WARNING)<<"stream process faild";
            return;
        }


//        if(!streamprocess_->ProcessStream(stream)){
//            //talk_base::Thread::Current()->Post(NULL,kaerp2p::MSG_DONE);
//            LOG(WARNING)<<"stream process faild";
//            return;
//        }


    std::string* msg = new std::string(writer.write(jmessage));
    LOG(INFO) <<"session sdp is " << *msg;
    this->UIThreadCallback(SEND_MESSAGE_TO_PEER,msg);
}

void ServerConductor::OnFailure(const std::string &error)
{
    LOG(LERROR) <<__FUNCTION__<< error;
}



void ServerConductor::OnIceCandidate(const IceCandidateInterface *candidate)
{
    LOG(INFO) << __FUNCTION__ << " " << candidate->sdp_mline_index();
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

    std::string* msg = new std::string(writer.write(jmessage));
    this->UIThreadCallback(SEND_MESSAGE_TO_PEER,msg);
}

void ServerConductor::OnMessage(talk_base::Message *msg)
{
    switch (msg->message_id) {
    case PEER_CONNECTION_CLOSED:
        LOG(INFO) << "PEER_CONNECTION_CLOSED";
        DeletePeerConnection();

        //ASSERT(active_streams_.empty());
        //        if (main_wnd_->IsWindow()) {
        //            if (client_->is_connected()) {
        //                main_wnd_->SwitchToPeerList(client_->peers());
        //            } else {
        //                main_wnd_->SwitchToConnectUI();
        //            }
        //        } else {
        //            DisconnectFromServer();
        //        }
        break;

    case SEND_MESSAGE_TO_PEER: {
        LOG(INFO) << "SEND_MESSAGE_TO_PEER";

        //std::string* msg = reinterpret_cast<std::string*>(data);
        talk_base::TypedMessageData<std::string *> *msgData =
                static_cast< talk_base::TypedMessageData<std::string *> *>(msg->pdata);
        if (msgData) {
            // For convenience, we always run the message through the queue.
            // This way we can be sure that messages are sent to the server
            // in the same order they were signaled without much hassle.
            pending_messages_.push_back(msgData->data());
        }
        std::string* msgStr ;
        if (!pending_messages_.empty() && !client_->IsSendingMessage()) {
            msgStr = pending_messages_.front();
            pending_messages_.pop_front();

            if (!client_->SendToPeer(peer_id_, *msgStr) && peer_id_ != -1) {
                LOG(LS_ERROR) << "SendToPeer failed";
                DisconnectFromServer();
            }
            std::cout<<"send msg : "<< *msgStr <<std::endl;
            delete msgStr;
        }

        if (!peer_connection_.get())
            peer_id_ = -1;

        break;
    }

    case PEER_CONNECTION_ERROR:
        LOG(INFO)<<"Error an unknown error occurred";
        break;

    case NEW_STREAM_ADDED: {
        break;
    }

    case STREAM_REMOVED: {
        break;
    }

    default:
        ASSERT(false);
        break;
    }
}

void ServerConductor::OnSignedIn()
{
    LOG(INFO) << __FUNCTION__;
    LOG(INFO) << "current id is "<<client_->id();
}

void ServerConductor::OnDisconnected()
{
    LOG(INFO) << __FUNCTION__;

    DeletePeerConnection();
}

void ServerConductor::OnPeerConnected(int id, const std::string &name)
{
    LOG(INFO) << __FUNCTION__;
    LOG(INFO) << "peer id = "<<id<<" ; peer name = "<<name;

    //        if(peer_id_ == -1 && client_->id() < id ){
    //            ConnectToPeer(id);
    //        }
}

void ServerConductor::OnPeerDisconnected(int peer_id)
{
    LOG(INFO) << __FUNCTION__;
    LOG(INFO) << "peer id = "<<peer_id;
    if (peer_id == peer_id_) {
        DeletePeerConnection();
    }
}

void ServerConductor::OnMessageFromPeer(int peer_id, const std::string &message)
{
    LOG(INFO) << __FUNCTION__;
    ASSERT(peer_id_ == peer_id || peer_id_ == -1);
    ASSERT(!message.empty());

    if (!peer_connection_.get()) {
        ASSERT(peer_id_ == -1);
        peer_id_ = peer_id;

        if (!InitializePeerConnection()) {
            LOG(LS_ERROR) << "Failed to initialize our PeerConnection instance";
            client_->SignOut();
            return;
        }
    } else if (peer_id != peer_id_) {
        ASSERT(peer_id_ != -1);
        LOG(WARNING) << "Received a message from unknown peer while already in a "
                        "conversation with a different peer.";
        return;
    }

    Json::Reader reader;
    Json::Value jmessage;
    if (!reader.parse(message, jmessage)) {
        LOG(WARNING) << "Received unknown message. " << message;
        return;
    }
    std::string type;
    std::string json_object;

    GetStringFromJsonObject(jmessage, kSessionDescriptionTypeName, &type);
    if (!type.empty()) {
        std::string sdp;
        if (!GetStringFromJsonObject(jmessage, kSessionDescriptionSdpName, &sdp)) {
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
        peer_connection_->SetRemoteDescription(DummySetSessionDescriptionObserver::Create(),session_description);
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

void ServerConductor::OnMessageSent(int err)
{
    LOG(INFO) <<__FUNCTION__;

    this->UIThreadCallback(SEND_MESSAGE_TO_PEER, 0);
}

void ServerConductor::OnServerConnectionFailure()
{
    LOG(INFO) <<__FUNCTION__;
}

}
