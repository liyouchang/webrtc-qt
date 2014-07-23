#ifndef KAERSESSION_H
#define KAERSESSION_H

#include "talk/p2p/base/session.h"
#include "talk/app/webrtc/jsep.h"
#include "talk/session/tunnel/pseudotcpchannel.h"

using namespace webrtc;

namespace kaerp2p {

class IceRestartAnswerLatch;
class KaerSessionDescriptionFactory;

// ICE state callback interface.
class IceObserver {
public:
    enum IceGatheringState {
        kIceGatheringNew,
        kIceGatheringGathering,
        kIceGatheringComplete
    };

    enum IceConnectionState {
        kIceConnectionNew,
        kIceConnectionChecking,
        kIceConnectionConnected,
        kIceConnectionCompleted,
        kIceConnectionFailed,
        kIceConnectionDisconnected,
        kIceConnectionClosed,
    };

public:
    IceObserver() {}
    // Called any time the IceConnectionState changes
    virtual void OnIceConnectionChange(IceConnectionState new_state) {}
    // Called any time the IceGatheringState changes
    virtual void OnIceGatheringChange(IceGatheringState new_state) {}
    // New Ice candidate have been found.
    virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) = 0;
    // All Ice candidates have been found.
    // TODO(bemasc): Remove this once callers transition to OnIceGatheringChange.
    // (via PeerConnectionObserver)
    virtual void OnIceComplete() {}

protected:
    ~IceObserver() {}

private:
    DISALLOW_COPY_AND_ASSIGN(IceObserver);
};

class KaerSession:public cricket::BaseSession
{
public:
    KaerSession(talk_base::Thread *signaling_thread,
                talk_base::Thread *worker_thread,
                cricket::PortAllocator *port_allocator);
    virtual ~KaerSession();

    bool Initialize();
    void Terminate();

    virtual talk_base::StreamInterface* GetStream();
    //cricket::PseudoTcpChannel * GetChannel();

    void RegisterIceObserver(IceObserver* observer);

    void CreateOffer(webrtc::CreateSessionDescriptionObserver* observer,
                     const std::string &description);
    void CreateAnswer(webrtc::CreateSessionDescriptionObserver* observer);

    // The ownership of |desc| will be transferred after this call.
    bool SetLocalDescription(webrtc::SessionDescriptionInterface* desc,
                             std::string* err_desc);
    // The ownership of |desc| will be transferred after this call.
    bool SetRemoteDescription(webrtc::SessionDescriptionInterface* desc,
                              std::string* err_desc);
    //add ice candidate     -- lht
    bool ProcessIceMessage(const webrtc::IceCandidateInterface* candidate);

    bool IceRestartPending() const;

    void ResetIceRestartLatch();

protected:
    bool CreateChannels(const cricket::SessionDescription* desc);
    bool CreatePseudoTcpChannel_s();

private:
    enum Action  {
        kOffer,
        kPrAnswer,
        kAnswer,
    };
    static Action GetAction(const std::string& type);

    void RemoveUnusedChannelsAndTransports(const cricket::SessionDescription* desc);
    bool UpdateSessionState(Action action, cricket::ContentSource source,
                            const cricket::SessionDescription* desc,
                            std::string* err_desc);
    void EnableChannels();
    bool StartCandidatesAllocation();
    // Copy the candidates from |saved_candidates_| to |dest_desc|.
    // The |saved_candidates_| will be cleared after this function call.
    void CopySavedCandidates(SessionDescriptionInterface* dest_desc);
    // Uses all remote candidates in |remote_desc| in this session.
    bool UseCandidatesInSessionDescription(
            const SessionDescriptionInterface* remote_desc);
    // Uses |candidate| in this session.
    bool UseCandidate(const IceCandidateInterface* candidate);

    void SetIceConnectionState(IceObserver::IceConnectionState state);

    // Below methods are helper methods which verifies SDP.
    bool ValidateSessionDescription(
            const webrtc::SessionDescriptionInterface* sdesc,
            cricket::ContentSource source, std::string* error_desc);
    // Check if a call to SetLocalDescription is acceptable with |action|.
    bool ExpectSetLocalDescription(Action action);
    // Check if a call to SetRemoteDescription is acceptable with |action|.
    bool ExpectSetRemoteDescription(Action action);

    std::string BadStateErrMsg(const std::string& type, State state);



    bool CreateTransportProxies(const cricket::TransportInfos& tinfos);
    cricket::TransportInfos GetEmptyTransportInfos(
            const cricket::ContentInfos& contents) const;

    // Creates a JsepIceCandidate and adds it to the local session description
    // and notify observers. Called when a new local candidate have been found.
    void ProcessNewLocalCandidate(const std::string& content_name,
                                  const cricket::Candidates& candidates);
    // Returns the media index for a local ice candidate given the content name.
    // Returns false if the local session description does not have a media
    // content called  |content_name|.
    bool GetLocalCandidateMediaIndex(const std::string& content_name,
                                     int* sdp_mline_index);

    IceObserver* ice_observer_;
    IceObserver::IceConnectionState ice_connection_state_;
    talk_base::scoped_ptr<IceRestartAnswerLatch> ice_restart_latch_;

    cricket::PseudoTcpChannel* channel_;
    // Candidates that arrived before the remote description was set.
    std::vector<webrtc::IceCandidateInterface*> saved_candidates_;
    talk_base::scoped_ptr<KaerSessionDescriptionFactory> kaer_session_desc_factory_;

public:
    talk_base::scoped_ptr<webrtc::SessionDescriptionInterface> local_desc_;
    talk_base::scoped_ptr<webrtc::SessionDescriptionInterface> remote_desc_;


    // BaseSession interface
protected:
    void OnTransportRequestSignaling(cricket::Transport *transport);
    void OnTransportConnecting(cricket::Transport *transport);
    void OnTransportWritable(cricket::Transport *transport);
    void OnTransportProxyCandidatesReady(
            cricket::TransportProxy *proxy,
            const cricket::Candidates &candidates);
    void OnCandidatesAllocationDone();
};


}
#endif // KAERSESSION_H
