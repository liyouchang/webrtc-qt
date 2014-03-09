#ifndef PEERTUNNEL_H
#define PEERTUNNEL_H

#include "talk/app/kaerp2p/KaerSession.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/proxy.h"
namespace kaerp2p{


class PeerTunnelInterface:public talk_base::RefCountInterface
{
public:
    enum SignalingState {
        kStable,
        kHaveLocalOffer,
        kHaveLocalPrAnswer,
        kHaveRemoteOffer,
        kHaveRemotePrAnswer,
        kClosed,
    };


    struct IceServer {
        std::string uri;
        std::string username;
        std::string password;
    };

    typedef std::vector<IceServer> IceServers;
    virtual talk_base::StreamInterface* GetStream() = 0;

    virtual const SessionDescriptionInterface* local_description() const = 0;
    virtual const SessionDescriptionInterface* remote_description() const  =0 ;

    // Create a new offer.
    // The CreateSessionDescriptionObserver callback will be called when done.
    virtual void CreateOffer(CreateSessionDescriptionObserver* observer) =0;
    // Create an answer to an offer.
    // The CreateSessionDescriptionObserver callback will be called when done.
    virtual void CreateAnswer(CreateSessionDescriptionObserver* observer) =0;
    // Sets the local session description.
    // JsepInterface takes the ownership of |desc| even if it fails.
    // The |observer| callback will be called when done.
    virtual void SetLocalDescription(SetSessionDescriptionObserver* observer,
                                     SessionDescriptionInterface* desc) =0;
    // Sets the remote session description.
    // JsepInterface takes the ownership of |desc| even if it fails.
    // The |observer| callback will be called when done.
    virtual void SetRemoteDescription(SetSessionDescriptionObserver* observer,
                                      SessionDescriptionInterface* desc) = 0;

    virtual bool AddIceCandidate(const IceCandidateInterface* ice_candidate) =0;

    virtual void Close() =0;

    virtual SignalingState signaling_state() = 0;
    virtual IceObserver::IceConnectionState ice_connection_state() = 0;
    virtual IceObserver::IceGatheringState ice_gathering_state() = 0;

protected:
    // Dtor protected as objects shouldn't be deleted via this interface.
    ~PeerTunnelInterface() {}

};

class PeerTunnelObserver {
public:
    virtual void OnError() = 0;
    // Triggered when the SignalingState changed.
    virtual void OnSignalingChange(PeerTunnelInterface::SignalingState new_state) {}

    // Triggered when renegotiation is needed, for example the ICE has restarted.
    virtual void OnRenegotiationNeeded() = 0;
    // Called any time the IceConnectionState changes
    virtual void OnIceConnectionChange(IceObserver::IceConnectionState new_state) {}

    // Called any time the IceGatheringState changes
    virtual void OnIceGatheringChange(IceObserver::IceGatheringState new_state) {}

    // New Ice candidate have been found.
    virtual void OnIceCandidate(const IceCandidateInterface* candidate) = 0;

protected:
    // Dtor protected as objects shouldn't be deleted via this interface.
    ~PeerTunnelObserver() {}
};

class PeerTunnel: public IceObserver,
        public talk_base::MessageHandler,
        public sigslot::has_slots<>,
        public PeerTunnelInterface
{
public:
    PeerTunnel();
    PeerTunnel(talk_base::Thread *signaling_thread ,
               talk_base::Thread *worker_thread);


    virtual talk_base::StreamInterface* GetStream();

    bool Initialize(const IceServers& configuration,PeerTunnelObserver* observer,
                    talk_base::Thread * stream_thread);
    talk_base::Thread* signaling_thread() const {
        return signaling_thread_;
    }

    virtual const SessionDescriptionInterface* local_description() const;
    virtual const SessionDescriptionInterface* remote_description() const ;

    // Create a new offer.
    // The CreateSessionDescriptionObserver callback will be called when done.
    virtual void CreateOffer(CreateSessionDescriptionObserver* observer) ;
    // Create an answer to an offer.
    // The CreateSessionDescriptionObserver callback will be called when done.
    virtual void CreateAnswer(CreateSessionDescriptionObserver* observer) ;
    // Sets the local session description.
    // JsepInterface takes the ownership of |desc| even if it fails.
    // The |observer| callback will be called when done.
    virtual void SetLocalDescription(SetSessionDescriptionObserver* observer,
                                     SessionDescriptionInterface* desc) ;
    // Sets the remote session description.
    // JsepInterface takes the ownership of |desc| even if it fails.
    // The |observer| callback will be called when done.
    virtual void SetRemoteDescription(SetSessionDescriptionObserver* observer,
                                      SessionDescriptionInterface* desc);


    virtual bool AddIceCandidate(const IceCandidateInterface* ice_candidate);

    virtual void Close();

    virtual SignalingState signaling_state();
    virtual IceConnectionState ice_connection_state();
    virtual IceGatheringState ice_gathering_state() ;

protected:

    void PostSetSessionDescriptionFailure(SetSessionDescriptionObserver* observer,
                                          const std::string& error);

    virtual void OnMessage(talk_base::Message* msg);


    // IceObserver interface
    void OnIceConnectionChange(IceConnectionState new_state);
    void OnIceGatheringChange(IceGatheringState new_state);
    void OnIceCandidate(const IceCandidateInterface *candidate);

    bool IsClosed() const {
        return signaling_state_ == kClosed;
    }
    void OnSessionStateChange(cricket::BaseSession* session,
                              cricket::BaseSession::State state);
    void ChangeSignalingState(SignalingState signaling_state);
private:
    talk_base::scoped_ptr<cricket::PortAllocator> port_allocator_;
    talk_base::scoped_ptr<KaerSession> session_;
    talk_base::Thread* signaling_thread_;
    talk_base::Thread* worker_thread_;
    talk_base::scoped_refptr<webrtc::PortAllocatorFactoryInterface> allocator_factory_;
    PeerTunnelObserver* observer_;

    SignalingState signaling_state_;
    IceConnectionState ice_connection_state_;
    IceGatheringState ice_gathering_state_;

};



// Define proxy for PeerConnectionInterface.
BEGIN_PROXY_MAP(PeerTunnel)
    PROXY_CONSTMETHOD0(const SessionDescriptionInterface*, local_description)
    PROXY_CONSTMETHOD0(const SessionDescriptionInterface*, remote_description)
    PROXY_METHOD1(void, CreateOffer, CreateSessionDescriptionObserver*)
    PROXY_METHOD1(void, CreateAnswer, CreateSessionDescriptionObserver*)
    PROXY_METHOD2(void, SetLocalDescription, SetSessionDescriptionObserver*,
                  SessionDescriptionInterface*)
    PROXY_METHOD2(void, SetRemoteDescription, SetSessionDescriptionObserver*,
                  SessionDescriptionInterface*)
    PROXY_METHOD1(bool, AddIceCandidate, const IceCandidateInterface*)
    PROXY_METHOD0(SignalingState, signaling_state)
    PROXY_METHOD0(IceObserver::IceConnectionState, ice_connection_state)
    PROXY_METHOD0(IceObserver::IceGatheringState, ice_gathering_state)
    PROXY_METHOD0(void, Close)
    PROXY_METHOD0(talk_base::StreamInterface* , GetStream)
END_PROXY()


}
#endif // PEERTUNNEL_H
