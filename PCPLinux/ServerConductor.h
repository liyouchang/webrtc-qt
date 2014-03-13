#ifndef MYCONDUCTOR_H
#define MYCONDUCTOR_H

#include "talk/app/kaerp2p/KaerSession.h"
#include "peer_connection_client.h"
#include <iostream>
#include "streamprocess.h"
#include <list>
#include "peertunnel.h"
namespace kaerp2p {



class ServerConductor:
        public webrtc::CreateSessionDescriptionObserver,
        public PeerTunnelObserver,
        public PeerConnectionClientObserver,
        public talk_base::MessageHandler
{
public:
    enum CallbackID {
        MEDIA_CHANNELS_INITIALIZED = 1,
        PEER_CONNECTION_CLOSED,
        SEND_MESSAGE_TO_PEER,
        PEER_CONNECTION_ERROR,
        NEW_STREAM_ADDED,
        STREAM_REMOVED,
    };

    ServerConductor(PeerConnectionClient* client);
    ~ServerConductor();
    virtual void StartLogin(const std::string& server, int port);
    virtual void DisconnectFromServer();

    virtual void ConnectToPeer(int peer_id);
    virtual void DisconnectFromCurrentPeer();
    virtual StreamProcess * GetStreamProcess();


protected:
    bool InitializePeerConnection();
    void DeletePeerConnection();
    int peer_id_;
    PeerConnectionClient* client_;
    std::deque<std::string*> pending_messages_;
    std::string server_;
    talk_base::scoped_refptr<PeerTunnelInterface> peer_connection_;

    StreamProcess * streamprocess_;
    std::list<int> other_peer_id_list_;
    talk_base::Thread * stream_thread_;
    talk_base::Thread * client_thread_;

    // CreateSessionDescriptionObserver interface
public:
    void OnSuccess(webrtc::SessionDescriptionInterface* desc);
    void OnFailure(const std::string &error);

public:
    void OnSignedIn();
    void OnDisconnected();
    void OnPeerConnected(int id, const std::string &name);
    void OnPeerDisconnected(int peer_id);
    void OnMessageFromPeer(int peer_id, const std::string &message);
    void OnMessageSent(int err);
    void OnServerConnectionFailure();

    void UIThreadCallback(int msg_id, void* data);


    // PeerTunnelObserver interface
public:
    void OnError();
    void OnRenegotiationNeeded(){}
    void OnIceCandidate(const IceCandidateInterface *candidate);


    // PeerTunnelObserver interface

    // MessageHandler interface
public:
    void OnMessage(talk_base::Message *msg);
};



}
#endif // MYCONDUCTOR_H
