#ifndef P2PCONDUCTOR_H
#define P2PCONDUCTOR_H

#include "talk/app/kaerp2p/KaerSession.h"
#include "peer_connection_client.h"
#include <iostream>
#include "streamprocess.h"
#include <list>
#include "peertunnel.h"


namespace kaerp2p {


struct PeerConnectionClientObserver {
  virtual void OnSignedIn() = 0;  // Called when we're logged on.
  virtual void OnDisconnected() = 0;
  virtual void OnPeerConnected(int id, const std::string& name) = 0;
  virtual void OnPeerDisconnected(const std::string& peer_id) = 0;
  virtual void OnMessageFromPeer(const std::string& peer_id, const std::string& message) = 0;
  virtual void OnMessageSent(int err) = 0;
  virtual void OnServerConnectionFailure() = 0;

 protected:
  virtual ~PeerConnectionClientObserver() {}
};

class PeerConnectionClientInterface
{
public:
    virtual bool SendToPeer(std::string peer_id, const std::string& message) =0;
    virtual bool SendHangUp(std::string peer_id) =0;
    virtual bool IsSendingMessage() = 0;
    virtual void OnReadPeerData(void * buffer,size_t len) = 0;
    virtual void OnP2PMessage(int msg_id) = 0;
    virtual void RegisterObserver(PeerConnectionClientObserver * callback){
        ASSERT(!callback_);
        callback_ = callback;
    }
    virtual const std::string & id() const {return my_id_;}


protected:
    std::string my_id_;

    PeerConnectionClientObserver* callback_;
    virtual ~PeerConnectionClientInterface() {}

};


class P2PConductor:
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


    P2PConductor(PeerConnectionClientInterface * client);

    virtual void ConnectToPeer(const std::string & peer_id);
    virtual void DisconnectFromCurrentPeer();
    virtual StreamProcess *GetStreamProcess();
protected:
    bool InitializePeerConnection();
    void DeletePeerConnection();
    void ClientThreadCallback(int msg_id, void* data);

    talk_base::scoped_refptr<PeerTunnelInterface> peer_connection_;
    PeerConnectionClientInterface * client_;
    std::deque<std::string*> pending_messages_;
    talk_base::Thread * stream_thread_;
    talk_base::Thread * client_thread_;
    StreamProcess * streamprocess_;
    std::string peer_id_;
    // CreateSessionDescriptionObserver interface
public:
    void OnSuccess(SessionDescriptionInterface *desc);
    void OnFailure(const std::string &error);

    // PeerTunnelObserver interface
public:
    void OnError();
    void OnRenegotiationNeeded(){};
    void OnIceCandidate(const IceCandidateInterface *candidate);

    // MessageHandler interface
public:
    void OnMessage(talk_base::Message *msg);

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
