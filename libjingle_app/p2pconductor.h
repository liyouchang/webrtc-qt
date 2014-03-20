#ifndef P2PCONDUCTOR_H
#define P2PCONDUCTOR_H

#include "talk/app/kaerp2p/KaerSession.h"
#include <iostream>
#include "streamprocess.h"
#include <list>
#include "talk/app/kaerp2p/peertunnel.h"
#include "PeerConnectionClinetInterface.h"
namespace kaerp2p {


class P2PConductor:
        public webrtc::CreateSessionDescriptionObserver,
        public PeerTunnelObserver,
        public PeerConnectionClientObserver
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


    P2PConductor(PeerConnectionClientInterface * client);
    ~P2PConductor();
    virtual void ConnectToPeer(const std::string & peer_id);
    virtual void DisconnectFromCurrentPeer();
    virtual StreamProcess *GetStreamProcess();
    void OnTunnelEstablished();
    virtual std::string GetPeerID(){ return peer_id_;}

    sigslot::signal1<StreamProcess *> SignalStreamOpened;
protected:
    bool InitializePeerConnection();
    void DeletePeerConnection();

    talk_base::scoped_refptr<PeerTunnelInterface> peer_connection_;
    PeerConnectionClientInterface * client_;
    talk_base::Thread * stream_thread_;
    talk_base::Thread * signal_thread_;
    talk_base::scoped_ptr<StreamProcess> stream_process_;
    bool tunnel_established_;

    //maybe conflict
    std::string peer_id_;

    // CreateSessionDescriptionObserver interface
public:
    void OnSuccess(SessionDescriptionInterface *desc);
    void OnFailure(const std::string &error);

    // PeerTunnelObserver interface
public:
    void OnError();
    void OnRenegotiationNeeded();
    void OnIceCandidate(const IceCandidateInterface *candidate);


    // PeerConnectionClientObserver interface
public:
    void OnSignedIn();
    void OnDisconnected();
    void OnPeerConnected(const std::string& id, const std::string &name);
    void OnPeerDisconnected(const std::string& peer_id);
    void OnMessageFromPeer(const std::string& peer_id, const std::string &message);
    void OnMessageSent(int err);
    void OnServerConnectionFailure();
};

}
#endif // P2PCONDUCTOR_H
