/*
* libjingle
* Copyright 2012, Google Inc.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  1. Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*  3. The name of the author may not be used to endorse or promote products
*     derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "conductor.h"

#include <utility>

//#include "talk/app/webrtc/videosourceinterface.h"
#include "talk/base/common.h"
#include "talk/base/json.h"
#include "talk/base/logging.h"
#include "defaults.h"
//#include "talk/media/devices/devicemanager.h"
#include "talk/p2p/client/basicportallocator.h"

#include "talk/app/webrtc/portallocatorfactory.h"

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


Conductor::Conductor(PeerConnectionClient* client, MainWindowInterface* main_wnd)
    : peer_id_(-1),
      client_(client),
      main_wnd_(main_wnd),
      fileProcess(NULL)
{
    client_->RegisterObserver(this);
    main_wnd->RegisterObserver(this);
}

Conductor::~Conductor() {
}

void Conductor::Close() {
    client_->SignOut();
    DeletePeerConnection();
}

bool Conductor::InitializePeerConnection()
{

    allocator_.reset(new cricket::BasicPortAllocator(
                         &network_manager_, stun_addr, talk_base::SocketAddress(),
                         talk_base::SocketAddress(), talk_base::SocketAddress()));

    allocator_->set_flags(cricket::PORTALLOCATOR_ENABLE_BUNDLE |
                               cricket::PORTALLOCATOR_ENABLE_SHARED_UFRAG |
                               cricket::PORTALLOCATOR_ENABLE_SHARED_SOCKET);

//    IceServers servers;
//    IceServer server;
//    server.uri = GetPeerConnectionString();
//    servers.push_back(server);


    session_.reset( new kaerp2p::KaerSession(talk_base::Thread::Current(),
                                             talk_base::Thread::Current(),
                                             allocator_.get(),
                                             "http://www.google.com/talk/tunnel")    );
    session_->RegisterIceObserver(this);

    return true;
}

void Conductor::DeletePeerConnection() {
    peer_id_ = -1;
}



//
// PeerConnectionObserver implementation.
//
void Conductor::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
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
    SendUIMessage(writer.write(jmessage));
}

//
// PeerConnectionClientObserver implementation.
//

void Conductor::OnSignedIn() {
    LOG(INFO) << __FUNCTION__;
    main_wnd_->SwitchToPeerList(client_->peers());
}

void Conductor::OnDisconnected() {
    LOG(INFO) << __FUNCTION__;

    DeletePeerConnection();

    if (main_wnd_->IsWindow())
        main_wnd_->SwitchToConnectUI();
}

void Conductor::OnPeerConnected(int id, const std::string& name) {
    LOG(INFO) << __FUNCTION__;
    // Refresh the list if we're showing it.
    if (main_wnd_->current_ui() == MainWindowInterface::LIST_PEERS)
        main_wnd_->SwitchToPeerList(client_->peers());
}

void Conductor::OnPeerDisconnected(int id) {
    LOG(INFO) << __FUNCTION__;
    if (id == peer_id_) {
        LOG(INFO) << "Our peer disconnected";
        main_wnd_->QueueUIThreadCallback(PEER_CONNECTION_CLOSED, NULL);
    } else {
        // Refresh the list if we're showing it.
        if (main_wnd_->current_ui() == MainWindowInterface::LIST_PEERS)
            main_wnd_->SwitchToPeerList(client_->peers());
    }
}

void Conductor::OnMessageFromPeer(int peer_id, const std::string& message) {
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
        session_->SetRemoteDescription(session_description,NULL);
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
        if (!session_->ProcessIceMessage(candidate.get())) {
            LOG(WARNING) << "Failed to apply the received candidate";
            return;
        }
        LOG(INFO) << " Received candidate :" << message;
        return;
    }
}

void Conductor::OnMessageSent(int err) {
    // Process the next pending message if any.
    main_wnd_->QueueUIThreadCallback(SEND_MESSAGE_TO_PEER, NULL);
}

void Conductor::OnServerConnectionFailure() {
    main_wnd_->ShowMessageBox("Error", ("Failed to connect to " + server_).c_str(),
                              true);
}

//
// MainWndCallback implementation.
//

void Conductor::StartLogin(const std::string& server, int port) {
    if (client_->is_connected())
        return;
    server_ = server;
    client_->Connect(server, port, GetPeerName());
}

void Conductor::DisconnectFromServer() {
    if (client_->is_connected())
        client_->SignOut();
}

void Conductor::ConnectToPeer(int peer_id) {
    ASSERT(peer_id_ == -1);
    ASSERT(peer_id != -1);

    if (session_.get()) {
        main_wnd_->ShowMessageBox("Error",
                                  "We only support connecting to one peer at a time", true);
        LOG(LS_INFO) <<"error";
        return;
    }

    if (InitializePeerConnection()) {
        peer_id_ = peer_id;
        session_->CreateOffer(this);
    } else {
        LOG(LS_INFO) <<"error";
        main_wnd_->ShowMessageBox("Error", "Failed to initialize PeerConnection", true);
    }
}


void Conductor::DisconnectFromCurrentPeer() {
    LOG(INFO) << __FUNCTION__;
    if (session_.get()) {
        client_->SendHangUp(peer_id_);
        DeletePeerConnection();
    }

    if (main_wnd_->IsWindow())
        main_wnd_->SwitchToPeerList(client_->peers());
}

void Conductor::UIThreadCallback(int msg_id, void* data) {
    switch (msg_id) {
    case PEER_CONNECTION_CLOSED:
        LOG(INFO) << "PEER_CONNECTION_CLOSED";
        DeletePeerConnection();


        if (main_wnd_->IsWindow()) {
            if (client_->is_connected()) {
                main_wnd_->SwitchToPeerList(client_->peers());
            } else {
                main_wnd_->SwitchToConnectUI();
            }
        } else {
            DisconnectFromServer();
        }
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
        main_wnd_->ShowMessageBox("Error", "an unknown error occurred", true);
        break;

    case NEW_STREAM_ADDED: {

        break;
    }

    case STREAM_REMOVED: {
        // Remote peer stopped sending a stream.
        webrtc::MediaStreamInterface* stream =
                reinterpret_cast<webrtc::MediaStreamInterface*>(
                    data);
        stream->Release();
        break;
    }

    default:
        ASSERT(false);
        break;
    }
}

void Conductor::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
    LOG(INFO) << __FUNCTION__;
    session_->SetLocalDescription(desc,NULL);

    talk_base::StreamInterface* stream = session_->GetStream();
    if(!fileProcess)
        fileProcess = new ClientFileProcess();
    fileProcess->ProcessStream(stream,"test",false);


    Json::StyledWriter writer;
    Json::Value jmessage;
    jmessage[kSessionDescriptionTypeName] = desc->type();
    std::string sdp;
    desc->ToString(&sdp);
    jmessage[kSessionDescriptionSdpName] = sdp;
    SendUIMessage(writer.write(jmessage));

    //LOG(WARNING) << "offer desc:" << sdp;
}

void Conductor::OnFailure(const std::string& error) {
    LOG(LERROR) << error;
}

void Conductor::SendUIMessage(const std::string& json_object)
{
    std::string* msg = new std::string(json_object);
    main_wnd_->QueueUIThreadCallback(SEND_MESSAGE_TO_PEER, msg);
}


bool ClientFileProcess::ProcessStream(talk_base::StreamInterface *stream, const std::string &filename, bool send) {
    //ASSERT(file_);
    sending_ = send;
    file_.reset(new talk_base::FileStream);
    buffer_len_ = 0;
    int err;
    if (!file_->Open(filename.c_str(), sending_ ? "rb" : "wb", &err)) {
        std::cerr << "Error opening <" << filename << ">: "
                  << std::strerror(err) << std::endl;
        return false;
    }
    stream->SignalEvent.connect(this, &ClientFileProcess::OnStreamEvent);
    if (stream->GetState() == talk_base::SS_CLOSED) {
        std::cerr << "Failed to establish P2P tunnel" << std::endl;
        return false;
    }
    if (stream->GetState() == talk_base::SS_OPEN) {
        OnStreamEvent(stream,
                      talk_base::SE_OPEN | talk_base::SE_READ | talk_base::SE_WRITE, 0);
    }
    return true;
}
