#ifndef P2PCONDUCTOR_H
#define P2PCONDUCTOR_H


#include <list>
#include <iostream>

#include "talk/app/kaerp2p/peertunnel.h"
#include "talk/base/messagehandler.h"

#include "streamprocess.h"
#include "PeerConnectionClinetInterface.h"


namespace kaerp2p {

//implament the p2p tunnel's create , write and read .
//we need to seperate the communication used by server and the communiction of p2p

//we use json to send the message of peer information
class P2PConductor:
        public webrtc::CreateSessionDescriptionObserver,
        public PeerTunnelObserver,
        public sigslot::has_slots<>,
        public talk_base::MessageHandler
{
public:
    P2PConductor();
    ~P2PConductor();
    virtual int ConnectToPeer(const std::string & peer_id);
    virtual void DisconnectFromCurrentPeer();

    virtual StreamProcess * GetStreamProcess();

    virtual std::string GetPeerID(){return peer_id_;}
    virtual void SetPeerID(std::string peer_id){peer_id_ = peer_id;}
    //if there are any message need to be handle by this peer ,use OnMessageFromPeer to process
    virtual void OnMessageFromPeer(const std::string& peer_id, const std::string &message);

    //we use SignalSendToPeer to send a sdp or candidates to peer which need to connect
    //peer_id and message
    sigslot::signal2<const std::string &, const std::string& > SignalNeedSendToPeer;
    //when connect success ,this signal will be emit
    sigslot::signal1<StreamProcess *> SignalStreamOpened;

protected:
    bool InitializePeerConnection();
    void DeletePeerConnection();

    void OnTunnelEstablished();

    talk_base::scoped_refptr<PeerTunnelInterface> peer_connection_;
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
    void OnIceGatheringChange(IceObserver::IceGatheringState new_state);

    // MessageHandler interface
public:
    enum {
        MSG_CONNECT_TIMEOUT
    };
    void OnMessage(talk_base::Message *msg);
};

}
#endif // P2PCONDUCTOR_H
