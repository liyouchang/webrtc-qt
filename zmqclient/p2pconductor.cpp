#include "p2pconductor.h"

#include "defaults.h"
#include "talk/p2p/client/basicportallocator.h"
#include "talk/base/json.h"
#include "talk/base/bind.h"

namespace  kaerp2p {


// Names used for a IceCandidate JSON object.
const char kCandidateSdpMidName[] = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[] = "candidate";

// Names used for a SessionDescription JSON object.
const char kSessionDescriptionTypeName[] = "type";
const char kSessionDescriptionSdpName[] = "sdp";



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

P2PConductor::P2PConductor(PeerConnectionClientInterface * client):
    client_(client),stream_thread_(NULL)
{
    client_->RegisterObserver(this);
    client_thread_ = talk_base::Thread::Current();

}

P2PConductor::~P2PConductor()
{
    if(stream_thread_)
        delete stream_thread_;

}


void P2PConductor::ConnectToPeer(const std::string &peer_id)
{
    if (peer_connection_.get()) {
        LOG(LS_INFO) <<"peer connection all ready connect";
        return;
    }
    if (InitializePeerConnection()) {
        peer_id_ = peer_id;
        peer_connection_->CreateOffer(this);
    } else {
        LOG(LS_INFO) <<"initialize connection error";
    }
}

void P2PConductor::DisconnectFromCurrentPeer()
{
    LOG(INFO) << __FUNCTION__;
    if (peer_connection_.get()) {
        client_->SendHangUp(peer_id_);
        DeletePeerConnection();
    }
}

StreamProcess *P2PConductor::GetStreamProcess()
{
    return streamprocess_;
}

void P2PConductor::OnTunnelEstablished()
{
    LOG(INFO) << __FUNCTION__;
    SignalStreamOpened(this->streamprocess_);
}


bool P2PConductor::InitializePeerConnection()
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

void P2PConductor::DeletePeerConnection()
{
    peer_connection_->Close();
    peer_connection_.release();
    peer_id_.clear();

}

void P2PConductor::ClientThreadCallback(int msg_id, void *data)
{
    switch (msg_id) {
    case PEER_CONNECTION_CLOSED:
        LOG(INFO) << "PEER_CONNECTION_CLOSED";
        DeletePeerConnection();
        break;

    case SEND_MESSAGE_TO_PEER: {
        //LOG(INFO) << "SEND_MESSAGE_TO_PEER";

        talk_base::TypedMessageData<std::string *> * msgData = NULL;

        std::string* msg = reinterpret_cast<std::string*>(data);
        if (msg) {
            msgData = new talk_base::TypedMessageData<std::string *>(msg);
        }
        client_thread_->Post(this,SEND_MESSAGE_TO_PEER,msgData);
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

void P2PConductor::OnSuccess(SessionDescriptionInterface *desc)
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
    if(!streamprocess_){
        streamprocess_ = new StreamProcess(stream_thread_);
        streamprocess_->SignalOpened.connect(this,&P2PConductor::OnTunnelEstablished);
    }
    bool result = stream_thread_->Invoke<bool>(
                talk_base::Bind(&StreamProcess::ProcessStream,streamprocess_,stream));
    if(!result){
        LOG(WARNING)<<"stream process faild";
        return;
    }

    std::string* msg = new std::string(writer.write(jmessage));
    LOG(INFO) <<"session sdp is " << *msg;
    this->ClientThreadCallback(SEND_MESSAGE_TO_PEER,msg);
}

void P2PConductor::OnFailure(const std::string &error)
{
    LOG(LERROR) <<__FUNCTION__<< error;
}

void P2PConductor::OnError()
{
    LOG(LS_ERROR) << __FUNCTION__ << "  offer error";
    DeletePeerConnection();
    //this->ClientThreadCallback(PEER_CONNECTION_ERROR, NULL);
}

void P2PConductor::OnRenegotiationNeeded()
{
    LOG(LS_ERROR) << __FUNCTION__;
}

void P2PConductor::OnIceCandidate(const IceCandidateInterface *candidate)
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
    this->ClientThreadCallback(SEND_MESSAGE_TO_PEER,msg);
}

void P2PConductor::OnMessage(talk_base::Message *msg)
{
    switch (msg->message_id) {
    case PEER_CONNECTION_CLOSED:
        LOG(INFO) << "PEER_CONNECTION_CLOSED";
        DeletePeerConnection();
        break;

    case SEND_MESSAGE_TO_PEER: {
        //LOG(INFO) << "SEND_MESSAGE_TO_PEER";

        //std::string* msg = reinterpret_cast<std::string*>(data);
        talk_base::TypedMessageData<std::string *> *msgData =
                static_cast< talk_base::TypedMessageData<std::string *> *>(msg->pdata);
        if (msgData) {
            // For convenience, we always run the message through the queue.
            // This way we can be sure that messages are sent to the server
            // in the same order they were signaled without much hassle.
            pending_messages_.push_back(msgData->data());
            delete msgData;
        }
        std::string* msgStr ;
        if (!pending_messages_.empty() && !client_->IsSendingMessage()) {
            msgStr = pending_messages_.front();
            pending_messages_.pop_front();

            if (!client_->SendToPeer(peer_id_, *msgStr) && !peer_id_.empty()) {
                LOG(LS_ERROR) << "SendToPeer failed";
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

void P2PConductor::OnSignedIn()
{
    LOG(INFO) << __FUNCTION__;
    LOG(INFO) << "current id is "<<client_->id();
}

void P2PConductor::OnDisconnected()
{
    LOG(INFO) << __FUNCTION__ << "disconnect from server";

    DeletePeerConnection();

}

void P2PConductor::OnPeerConnected(const std::string& id, const std::string &name)
{
    LOG(INFO) << __FUNCTION__ << " a peer connected server";
    LOG(INFO) << "peer id = "<<id<<" ; peer name = "<<name;

}

void P2PConductor::OnPeerDisconnected(const std::string& peer_id)
{
    LOG(INFO) << __FUNCTION__ << " a peer disconnected server";
    LOG(INFO) << "peer id = "<<peer_id;
    if (peer_id == peer_id_) {
        DeletePeerConnection();
    }
}

void P2PConductor::OnMessageFromPeer(const std::string &peer_id, const std::string &message)
{
    LOG(INFO) << __FUNCTION__;
    ASSERT(peer_id_ == peer_id || peer_id_ == -1);
    ASSERT(!message.empty());

    Json::Reader reader;
    Json::Value jmessage;
    if (!reader.parse(message, jmessage)) {
        LOG(WARNING) << "Received unknown message. " << message;
        return;
    }

    if (!peer_connection_.get()) {
        ASSERT(peer_id_ == -1);
        peer_id_ = peer_id;

        if (!InitializePeerConnection()) {
            LOG(LS_ERROR) << "Failed to initialize our PeerConnection instance";
            //client_->SignOut();
            return;
        }
    } else if (peer_id != peer_id_) {
        ASSERT(peer_id_ != -1);
        LOG(WARNING) << "Received a message from unknown peer while already in a "
                        "conversation with a different peer.";
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

void P2PConductor::OnMessageSent(int err)
{
    LOG(INFO) <<__FUNCTION__ << err;

    this->ClientThreadCallback(SEND_MESSAGE_TO_PEER, 0);

}

void P2PConductor::OnServerConnectionFailure()
{
    LOG(INFO) <<__FUNCTION__ ;

}

}
