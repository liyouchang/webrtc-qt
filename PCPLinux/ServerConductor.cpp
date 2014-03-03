
#include "ServerConductor.h"
#include "defaults.h"
#include "talk/p2p/client/basicportallocator.h"
#include "talk/base/json.h"

namespace kaerp2p {

// Names used for a IceCandidate JSON object.
const char kCandidateSdpMidName[] = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[] = "candidate";

// Names used for a SessionDescription JSON object.
const char kSessionDescriptionTypeName[] = "type";
const char kSessionDescriptionSdpName[] = "sdp";

struct IceServer {
    std::string uri;
    std::string username;
    std::string password;
};
typedef std::vector<IceServer> IceServers;

talk_base::SocketAddress stun_addr("stun.l.google.com", 19302);


ServerConductor::ServerConductor(PeerConnectionClient *client):
    peer_id_(-1),
    client_(client),
    fileProcess(NULL)
{
    client_->RegisterObserver(this);

    allocator_ = new cricket::BasicPortAllocator(
                &network_manager_, stun_addr, talk_base::SocketAddress(),
                talk_base::SocketAddress(), talk_base::SocketAddress());
    //    allocator_.reset(new cricket::BasicPortAllocator(
    //                         &network_manager_, stun_addr, talk_base::SocketAddress(),
    //                         talk_base::SocketAddress(), talk_base::SocketAddress()));

    allocator_->set_flags(cricket::PORTALLOCATOR_ENABLE_BUNDLE |
                          cricket::PORTALLOCATOR_ENABLE_SHARED_UFRAG |
                          cricket::PORTALLOCATOR_ENABLE_SHARED_SOCKET);


}

ServerConductor::~ServerConductor()
{
    delete allocator_;
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

    if (session_.get()) {
        //        main_wnd_->ShowMessageBox("Error",
        //                                  "We only support connecting to one peer at a time", true);
        LOG(LS_INFO) <<"error";
        return;
    }

    if (InitializePeerConnection()) {
        peer_id_ = peer_id;
        session_->CreateOffer(this);
    } else {
        LOG(LS_INFO) <<"error";
        //main_wnd_->ShowMessageBox("Error", "Failed to initialize PeerConnection", true);
    }
}

void ServerConductor::DisconnectFromCurrentPeer()
{
    LOG(INFO) << __FUNCTION__;
    if (session_.get()) {
        client_->SendHangUp(peer_id_);
        DeletePeerConnection();
    }

    //    if (main_wnd_->IsWindow())
    //        main_wnd_->SwitchToPeerList(client_->peers());
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
        std::string* msg = reinterpret_cast<std::string*>(data);
        if (msg) {
            // For convenience, we always run the message through the queue.
            // This way we can be sure that messages are sent to the server
            // in the same order they were signaled without much hassle.
            pending_messages_.push_back(msg);
        }

        if (!pending_messages_.empty() && !client_->IsSendingMessage()) {
            msg = pending_messages_.front();
            pending_messages_.pop_front();

            if (!client_->SendToPeer(peer_id_, *msg) && peer_id_ != -1) {
                LOG(LS_ERROR) << "SendToPeer failed";
                DisconnectFromServer();
            }
            delete msg;
        }

        if (!session_.get())
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

bool ServerConductor::InitializePeerConnection()
{
    IceServers servers;
    IceServer server;
    server.uri = GetPeerConnectionString();
    servers.push_back(server);


    session_.reset( new KaerSession(talk_base::Thread::Current(),
                                    talk_base::Thread::Current(),
                                    allocator_,
                                    "http://www.google.com/talk/tunnel")    );
    session_->RegisterIceObserver(this);

    return true;
}

void ServerConductor::DeletePeerConnection()
{
    session_.release();
    //active_streams_.clear();
    //main_wnd_->StopLocalRenderer();
    //main_wnd_->StopRemoteRenderer();
    //peer_connection_factory_ = NULL;
    peer_id_ = -1;
}

void ServerConductor::OnSuccess(SessionDescriptionInterface *desc)
{
    LOG(INFO) << __FUNCTION__;

    session_->SetLocalDescription(desc,NULL);
    Json::StyledWriter writer;
    Json::Value jmessage;
    jmessage[kSessionDescriptionTypeName] = desc->type();
    std::string sdp;
    desc->ToString(&sdp);
    jmessage[kSessionDescriptionSdpName] = sdp;

//    talk_base::StreamInterface* stream = session_->GetStream();
//    if(!fileProcess)
//        fileProcess = new ServerFileProcess();

//    fileProcess->ProcessStream(stream,"PeerClientTest",false);

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

        if(peer_id_ == -1 && client_->id() < id ){
            ConnectToPeer(id);
        }
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
    ASSERT(peer_id_ == peer_id || peer_id_ == -1);
    ASSERT(!message.empty());

    if (!session_.get()) {
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
        session_->SetRemoteDescription( session_description,NULL);
        if (session_description->type() ==
                webrtc::SessionDescriptionInterface::kOffer) {
            session_->CreateAnswer(this);
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

        if (!session_->ProcessIceMessage(candidate.get())) {
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
