#include "KaerSession.h"
#include "talk/app/webrtc/jsepsessiondescription.h"
#include "talk/app/kaerp2p/kaersessiondescriptionfactory.h"
#include "talk/p2p/base/constants.h"
#include "talk/p2p/base/sessiondescription.h"
#include "talk/base/stringencode.h"
#include "talk/session/tunnel/tunnelsessionclient.h"
#include "talk/base/bind.h"
#include "talk/session/tunnel/pseudotcpchannel.h"

#include "talk/app/kaerp2p/udpstreamchannel.h"

using cricket::ContentInfo;
using cricket::ContentInfos;
using cricket::SessionDescription;
using cricket::TransportInfo;


namespace kaerp2p {

const char NS_TUNNEL[] = "http://www.google.com/talk/tunnel";
const char CN_TUNNEL[] = "tunnel";

// Error messages
const char kSetLocalSdpFailed[] = "SetLocalDescription failed: ";
const char kSetRemoteSdpFailed[] = "SetRemoteDescription failed: ";
const char kCreateChannelFailed[] = "Failed to create channels.";
const char kBundleWithoutRtcpMux[] = "RTCP-MUX must be enabled when BUNDLE "
        "is enabled.";
const char kInvalidCandidates[] = "Description contains invalid candidates.";
const char kInvalidSdp[] = "Invalid session description.";
const char kMlineMismatch[] =
        "Offer and answer descriptions m-lines are not matching. "
        "Rejecting answer.";
const char kSdpWithoutCrypto[] = "Called with a SDP without crypto enabled.";
const char kSdpWithoutSdesAndDtlsDisabled[] =
        "Called with an SDP without SDES crypto and DTLS disabled locally.";
const char kSdpWithoutIceUfragPwd[] =
        "Called with an SDP without ice-ufrag and ice-pwd.";
const char kSessionError[] = "Session error code: ";
const char kUpdateStateFailed[] = "Failed to update session state: ";
const char kPushDownOfferTDFailed[] =
        "Failed to push down offer transport description.";
const char kPushDownPranswerTDFailed[] =
        "Failed to push down pranswer transport description.";
const char kPushDownAnswerTDFailed[] =
        "Failed to push down answer transport description.";

// Compares |answer| against |offer|. Comparision is done
// for number of m-lines in answer against offer. If matches true will be
// returned otherwise false.
static bool VerifyMediaDescriptions(
        const SessionDescription* answer, const SessionDescription* offer) {
    if (offer->contents().size() != answer->contents().size())
        return false;

    for (size_t i = 0; i < offer->contents().size(); ++i) {
        if ((offer->contents()[i].name) != answer->contents()[i].name) {
            return false;
        }
    }
    return true;
}


// Checks that each non-rejected content has ice-ufrag and ice-pwd set.
static bool VerifyIceUfragPwdPresent(const SessionDescription* desc) {
    const ContentInfos& contents = desc->contents();
    for (size_t index = 0; index < contents.size(); ++index) {
        const ContentInfo* cinfo = &contents[index];
        if (cinfo->rejected) {
            continue;
        }

        // If the content isn't rejected, ice-ufrag and ice-pwd must be present.
        const TransportInfo* tinfo = desc->GetTransportInfoByName(cinfo->name);
        if (!tinfo) {
            // Something is not right.
            LOG(LS_ERROR) << kInvalidSdp;
            return false;
        }
        if (tinfo->description.ice_ufrag.empty() ||
                tinfo->description.ice_pwd.empty()) {
            LOG(LS_ERROR) << "Session description must have ice ufrag and pwd.";
            return false;
        }
    }
    return true;
}




static bool BadSdp(const std::string& desc, std::string* err_desc) {
    if (err_desc) {
        *err_desc = desc;
    }
    LOG(LS_ERROR) << desc;
    return false;
}

static bool BadLocalSdp(const std::string& desc, std::string* err_desc) {
    std::string set_local_sdp_failed = kSetLocalSdpFailed;
    set_local_sdp_failed.append(desc);
    return BadSdp(set_local_sdp_failed, err_desc);
}

static bool BadRemoteSdp(const std::string& desc, std::string* err_desc) {
    std::string set_remote_sdp_failed = kSetRemoteSdpFailed;
    set_remote_sdp_failed.append(desc);
    return BadSdp(set_remote_sdp_failed, err_desc);
}

static bool BadSdp(cricket::ContentSource source,
                   const std::string& desc, std::string* err_desc) {
    if (source == cricket::CS_LOCAL) {
        return BadLocalSdp(desc, err_desc);
    } else {
        return BadRemoteSdp(desc, err_desc);
    }
}
static std::string SessionErrorMsg(cricket::BaseSession::Error error) {
    std::ostringstream desc;
    desc << kSessionError << error;
    return desc.str();
}
#define GET_STRING_OF_STATE(state)  \
    case cricket::BaseSession::state:  \
    result = #state;  \
    break;

static std::string GetStateString(cricket::BaseSession::State state) {
    std::string result;
    switch (state) {
    GET_STRING_OF_STATE(STATE_INIT)
            GET_STRING_OF_STATE(STATE_SENTINITIATE)
            GET_STRING_OF_STATE(STATE_RECEIVEDINITIATE)
            GET_STRING_OF_STATE(STATE_SENTPRACCEPT)
            GET_STRING_OF_STATE(STATE_SENTACCEPT)
            GET_STRING_OF_STATE(STATE_RECEIVEDPRACCEPT)
            GET_STRING_OF_STATE(STATE_RECEIVEDACCEPT)
            GET_STRING_OF_STATE(STATE_SENTMODIFY)
            GET_STRING_OF_STATE(STATE_RECEIVEDMODIFY)
            GET_STRING_OF_STATE(STATE_SENTREJECT)
            GET_STRING_OF_STATE(STATE_RECEIVEDREJECT)
            GET_STRING_OF_STATE(STATE_SENTREDIRECT)
            GET_STRING_OF_STATE(STATE_SENTTERMINATE)
            GET_STRING_OF_STATE(STATE_RECEIVEDTERMINATE)
            GET_STRING_OF_STATE(STATE_INPROGRESS)
            GET_STRING_OF_STATE(STATE_DEINIT)
            default:
        ASSERT(false);
    break;
    }
    return result;
}

#define GET_STRING_OF_ERROR(err)  \
    case cricket::BaseSession::err:  \
    result = #err;  \
    break;

static std::string GetErrorString(cricket::BaseSession::Error err) {
    std::string result;
    switch (err) {
    GET_STRING_OF_ERROR(ERROR_NONE)
            GET_STRING_OF_ERROR(ERROR_TIME)
            GET_STRING_OF_ERROR(ERROR_RESPONSE)
            GET_STRING_OF_ERROR(ERROR_NETWORK)
            GET_STRING_OF_ERROR(ERROR_CONTENT)
            GET_STRING_OF_ERROR(ERROR_TRANSPORT)
            default:
        ASSERT(false);
    break;
    }
    return result;
}

static bool SetSessionStateFailed(cricket::ContentSource source,
                                  cricket::BaseSession::Error err,
                                  std::string* err_desc) {
    std::string set_state_err = kUpdateStateFailed;
    set_state_err.append(GetErrorString(err));
    return BadSdp(source, set_state_err, err_desc);
}

// Help class used to remember if a a remote peer has requested ice restart by
// by sending a description with new ice ufrag and password.
class IceRestartAnswerLatch {
public:
    IceRestartAnswerLatch() : ice_restart_(false) { }

    // Returns true if CheckForRemoteIceRestart has been called with a new session
    // description where ice password and ufrag has changed since last time
    // Reset() was called.
    bool Get() const {
        return ice_restart_;
    }

    void Reset() {
        if (ice_restart_) {
            ice_restart_ = false;
        }
    }

    void CheckForRemoteIceRestart(
            const SessionDescriptionInterface* old_desc,
            const SessionDescriptionInterface* new_desc) {
        if (!old_desc || new_desc->type() != SessionDescriptionInterface::kOffer) {
            return;
        }
        const SessionDescription* new_sd = new_desc->description();
        const SessionDescription* old_sd = old_desc->description();
        const ContentInfos& contents = new_sd->contents();
        for (size_t index = 0; index < contents.size(); ++index) {
            const ContentInfo* cinfo = &contents[index];
            if (cinfo->rejected) {
                continue;
            }
            // If the content isn't rejected, check if ufrag and password has
            // changed.
            const cricket::TransportDescription* new_transport_desc =
                    new_sd->GetTransportDescriptionByName(cinfo->name);
            const cricket::TransportDescription* old_transport_desc =
                    old_sd->GetTransportDescriptionByName(cinfo->name);
            if (!new_transport_desc || !old_transport_desc) {
                // No transport description exist. This is not an ice restart.
                continue;
            }
            if (new_transport_desc->ice_pwd != old_transport_desc->ice_pwd &&
                    new_transport_desc->ice_ufrag != old_transport_desc->ice_ufrag) {
                LOG(LS_INFO) << "Remote peer request ice restart.";
                ice_restart_ = true;
                break;
            }
        }
    }

private:
    bool ice_restart_;
};


KaerSession::KaerSession(talk_base::Thread* signaling_thread,
                         talk_base::Thread* worker_thread,
                         cricket::PortAllocator* port_allocator):
    cricket::BaseSession(signaling_thread , worker_thread , port_allocator,
                         talk_base::ToString(talk_base::CreateRandomId64() &
                                             LLONG_MAX),
                         cricket::NS_TUNNEL,false),
    ice_connection_state_(IceObserver::kIceConnectionNew),
    ice_observer_(NULL),
    ice_restart_latch_(new IceRestartAnswerLatch),
    channel_(NULL)
{
    //    kaer_session_desc_factory_.reset(new KaerSessionDescriptionFactory(
    //                                         this->signaling_thread(),this,id()));
}

KaerSession::~KaerSession()
{
    Terminate();
    for (size_t i = 0; i < saved_candidates_.size(); ++i) {
        delete saved_candidates_[i];
    }
}

bool KaerSession::Initialize()
{
    kaer_session_desc_factory_.reset(new KaerSessionDescriptionFactory(
                                         signaling_thread(),this,id()));
    return true;
}

void KaerSession::Terminate()
{
    SetState(STATE_RECEIVEDTERMINATE);
    if(channel_ != NULL){
        const std::string content_name = channel_->content_name();
        DestroyTransportProxy(content_name);
    }
    //channel will destroy when transport destroyed
    //no need to call OnSessionTerminate
    //channel_->OnSessionTerminate(this);
}

talk_base::StreamInterface *KaerSession::GetStream()
{
    ASSERT(channel_ != NULL);
    return channel_->GetStream();
}

void KaerSession::RegisterIceObserver(IceObserver *observer) {
    ice_observer_ = observer;
}

void KaerSession::CreateOffer(webrtc::CreateSessionDescriptionObserver *observer,
                              const std::string &description)
{
    kaer_session_desc_factory_->CreateOffer(observer,description);
}

void KaerSession::CreateAnswer(webrtc::CreateSessionDescriptionObserver *observer)
{
    kaer_session_desc_factory_->CreateAnswer(observer);
}

bool KaerSession::SetLocalDescription(webrtc::SessionDescriptionInterface *desc,
                                      std::string *err_desc)
{
    // Takes the ownership of |desc| regardless of the result.
    talk_base::scoped_ptr<webrtc::SessionDescriptionInterface> desc_temp(desc);

    // Validate SDP.
    if (!ValidateSessionDescription(desc, cricket::CS_LOCAL, err_desc)) {
        return false;
    }

    // Update the initiator flag if this session is the initiator.
    Action action = GetAction(desc->type());
    if (state() == STATE_INIT && action == kOffer) {
        set_initiator(true);
    }

    //    cricket::SecureMediaPolicy secure_policy =
    //        webrtc_session_desc_factory_->Secure();
    //    // Update the MediaContentDescription crypto settings as per the policy set.
    //    UpdateSessionDescriptionSecurePolicy(secure_policy, desc->description());

    set_local_description(desc->description()->Copy());
    local_desc_.reset(desc_temp.release());

    // Transport and Media channels will be created only when offer is set.
    if (action == kOffer && !CreateChannels(local_desc_->description())) {
        // TODO(mallinath) - Handle CreateChannel failure, as new local description
        // is applied. Restore back to old description.
        return BadLocalSdp(kCreateChannelFailed, err_desc);
    }

    // Remove channel and transport proxies, if MediaContentDescription is
    // rejected.
    RemoveUnusedChannelsAndTransports(local_desc_->description());

    if (!UpdateSessionState(action, cricket::CS_LOCAL,
                            local_desc_->description(), err_desc)) {
        return false;
    }
    // Kick starting the ice candidates allocation.
    StartCandidatesAllocation();

    // Update state and SSRC of local MediaStreams and DataChannels based on the
    // local session description.
    //    mediastream_signaling_->OnLocalDescriptionChanged(local_desc_.get());

    //    talk_base::SSLRole role;
    //    if (data_channel_type_ == cricket::DCT_SCTP && GetSslRole(&role)) {
    //      mediastream_signaling_->OnDtlsRoleReadyForSctp(role);
    //    }

    if (error() != cricket::BaseSession::ERROR_NONE) {
        return BadLocalSdp(SessionErrorMsg(error()), err_desc);
    }

    return true;
}

bool KaerSession::SetRemoteDescription(SessionDescriptionInterface *desc,
                                       std::string *err_desc)
{
    // Takes the ownership of |desc| regardless of the result.
    talk_base::scoped_ptr<SessionDescriptionInterface> desc_temp(desc);

    // Validate SDP.
    if (!ValidateSessionDescription(desc, cricket::CS_REMOTE, err_desc)) {
        return false;
    }

    // Transport and Media channels will be created only when offer is set.
    Action action = GetAction(desc->type());
    if (action == kOffer && !CreateChannels(desc->description())) {
        // TODO(mallinath) - Handle CreateChannel failure, as new local description
        // is applied. Restore back to old description.
        return BadRemoteSdp(kCreateChannelFailed, err_desc);
    }

    // Remove channel and transport proxies, if MediaContentDescription is
    // rejected.
    RemoveUnusedChannelsAndTransports(desc->description());

    // NOTE: Candidates allocation will be initiated only when SetLocalDescription
    // is called.
    set_remote_description(desc->description()->Copy());
    if (!UpdateSessionState(action, cricket::CS_REMOTE,
                            desc->description(), err_desc)) {
        return false;
    }

    // Update remote MediaStreams.
    //    mediastream_signaling_->OnRemoteDescriptionChanged(desc);
    //    if (local_description() && !UseCandidatesInSessionDescription(desc)) {
    //      return BadRemoteSdp(kInvalidCandidates, err_desc);
    //    }

    // Copy all saved candidates.
    CopySavedCandidates(desc);
    // We retain all received candidates.
    KaerSessionDescriptionFactory::CopyCandidatesFromSessionDescription(
                remote_desc_.get(), desc);
    // Check if this new SessionDescription contains new ice ufrag and password
    // that indicates the remote peer requests ice restart.
    ice_restart_latch_->CheckForRemoteIceRestart(remote_desc_.get(),
                                                 desc);
    remote_desc_.reset(desc_temp.release());

    //    talk_base::SSLRole role;
    //    if (data_channel_type_ == cricket::DCT_SCTP && GetSslRole(&role)) {
    //      mediastream_signaling_->OnDtlsRoleReadyForSctp(role);
    //    }

    if (error() != cricket::BaseSession::ERROR_NONE) {
        return BadRemoteSdp(SessionErrorMsg(error()), err_desc);
    }
    return true;
}

bool KaerSession::ProcessIceMessage(const IceCandidateInterface *candidate)
{
    if (state() == STATE_INIT) {
        LOG(LS_ERROR) << "ProcessIceMessage: ICE candidates can't be added "
                      << "without any offer (local or remote) "
                      << "session description.";
        return false;
    }

    if (!candidate) {
        LOG(LS_ERROR) << "ProcessIceMessage: Candidate is NULL";
        return false;
    }

    if (!local_description() || !remote_description()) {
        LOG(LS_INFO) << "ProcessIceMessage: Remote description not set, "
                     << "save the candidate for later use.";
        saved_candidates_.push_back(
                    new JsepIceCandidate(candidate->sdp_mid(),
                                         candidate->sdp_mline_index(),
                                         candidate->candidate()));
        return true;
    }
    // Add this candidate to the remote session description.
    if (!remote_desc_->AddCandidate(candidate)) {
        LOG(LS_ERROR) << "ProcessIceMessage: Candidate cannot be used";
        return false;
    }

    return UseCandidatesInSessionDescription(remote_desc_.get());
}

bool KaerSession::IceRestartPending() const
{
    return ice_restart_latch_->Get();
}

void KaerSession::ResetIceRestartLatch()
{
    ice_restart_latch_->Reset();
}

KaerSession::Action KaerSession::GetAction(const std::string &type)
{
    if (type == SessionDescriptionInterface::kOffer) {
        return KaerSession::kOffer;
    } else if (type == SessionDescriptionInterface::kPrAnswer) {
        return KaerSession::kPrAnswer;
    } else if (type == SessionDescriptionInterface::kAnswer) {
        return KaerSession::kAnswer;
    }
    ASSERT(false && "unknown action type");
    return KaerSession::kOffer;
}

#ifdef ARM
const int kChannelWriteBufferSize = 256*1024;
const int kChannelReadBufferSize = 64*1024;
#else
const int kChannelWriteBufferSize = 512*1024;
const int kChannelReadBufferSize = 256*1024;
#endif

bool KaerSession::CreateChannels(const cricket::SessionDescription *desc)
{
    const ContentInfo* cinfo = desc->FirstContentByType(NS_TUNNEL);
    if(!cinfo){
        LOG_T_F(WARNING)<<"get tunnel info failed";
        return false;
    }
    const cricket::TunnelContentDescription* tunnel_desc =
            static_cast<const cricket::TunnelContentDescription*>(cinfo->description);
    LOG_T_F(INFO)<<"channel name is "<<tunnel_desc->description;
    //    return this->signaling_thread()->Invoke<bool>(
    //                talk_base::Bind(&KaerSession::CreatePseudoTcpChannel_s,this));
    cricket::PseudoTcpChannel * newChannel =
            new cricket::PseudoTcpChannel(this->worker_thread(), this);
    newChannel->Connect(CN_TUNNEL,"tcp", 1);
//    newChannel->SetOption(cricket::PseudoTcp::OPT_SNDBUF,kChannelWriteBufferSize);
//    newChannel->SetOption(cricket::PseudoTcp::OPT_RCVBUF,kChannelReadBufferSize);
    channel_ = newChannel;
//    channel_ = new kaerp2p::UdpStreamChannel(this->worker_thread(), this);
//    channel_->Connect(CN_TUNNEL,"udp",1);
    return true;
}

//bool KaerSession::CreatePseudoTcpChannel_s()
//{
//    channel_->Connect(CN_TUNNEL,"tcp", 1);
//    channel_->SetOption(cricket::PseudoTcp::OPT_SNDBUF,512*1024);
//    channel_->SetOption(cricket::PseudoTcp::OPT_RCVBUF,128*1024);
//    channel_->SignalChannelClosed.connect(this, &KaerSession::OnStreamChannelClosed);
//    //LOG(INFO)<<"KaerSession::CreatePseudoTcpChannel_s---channel option set";
//    return true;
//}

void KaerSession::RemoveUnusedChannelsAndTransports(
        const cricket::SessionDescription *desc)
{
    const ContentInfo* cinfo = desc->FirstContentByType(NS_TUNNEL);
    if( (!cinfo || cinfo->rejected) && channel_){
        const std::string content_name = channel_->content_name();
        DestroyTransportProxy(content_name);
    }
}

bool KaerSession::UpdateSessionState(KaerSession::Action action,
                                     cricket::ContentSource source,
                                     const cricket::SessionDescription *desc,
                                     std::string *err_desc)
{
    // If there's already a pending error then no state transition should happen.
    // But all call-sites should be verifying this before calling us!
    ASSERT(error() == cricket::BaseSession::ERROR_NONE);
    std::string td_err;
    if (action == kOffer) {
        if (!PushdownTransportDescription(source, cricket::CA_OFFER,&td_err)) {
            return BadSdp(source, kPushDownOfferTDFailed, err_desc);
        }
        SetState(source == cricket::CS_LOCAL ?
                     STATE_SENTINITIATE : STATE_RECEIVEDINITIATE);
        if (error() != cricket::BaseSession::ERROR_NONE) {
            return SetSessionStateFailed(source, error(), err_desc);
        }
    } else if (action == kPrAnswer) {
        if (!PushdownTransportDescription(source, cricket::CA_PRANSWER,&td_err)) {
            return BadSdp(source, kPushDownPranswerTDFailed, err_desc);
        }
        EnableChannels();
        SetState(source == cricket::CS_LOCAL ?
                     STATE_SENTPRACCEPT : STATE_RECEIVEDPRACCEPT);
        if (error() != cricket::BaseSession::ERROR_NONE) {
            return SetSessionStateFailed(source, error(), err_desc);
        }
    } else if (action == kAnswer) {
        if (!PushdownTransportDescription(source, cricket::CA_ANSWER,&td_err)) {
            return BadSdp(source, kPushDownAnswerTDFailed, err_desc);
        }
        MaybeEnableMuxingSupport();
        EnableChannels();
        SetState(source == cricket::CS_LOCAL ?
                     STATE_SENTACCEPT : STATE_RECEIVEDACCEPT);
        if (error() != cricket::BaseSession::ERROR_NONE) {
            return SetSessionStateFailed(source, error(), err_desc);
        }
    }
    return true;
}

void KaerSession::EnableChannels()
{
}

bool KaerSession::StartCandidatesAllocation()
{
    // SpeculativelyConnectTransportChannels, will call ConnectChannels method
    // from TransportProxy to start gathering ice candidates.
    SpeculativelyConnectAllTransportChannels();
    if (!saved_candidates_.empty()) {
        // If there are saved candidates which arrived before local description is
        // set, copy those to remote description.
        CopySavedCandidates(remote_desc_.get());
    }
    // Push remote candidates present in remote description to transport channels.
    UseCandidatesInSessionDescription(remote_desc_.get());
    return true;
}

void KaerSession::CopySavedCandidates(SessionDescriptionInterface *dest_desc)
{
    if (!dest_desc) {
        ASSERT(false);
        return;
    }
    for (size_t i = 0; i < saved_candidates_.size(); ++i) {
        dest_desc->AddCandidate(saved_candidates_[i]);
        delete saved_candidates_[i];
    }
    saved_candidates_.clear();
}

bool KaerSession::UseCandidatesInSessionDescription(
        const SessionDescriptionInterface *remote_desc)
{
    if (!remote_desc)
        return true;
    bool ret = true;
    for (size_t m = 0; m < remote_desc->number_of_mediasections(); ++m) {
        const IceCandidateCollection* candidates = remote_desc->candidates(m);
        for  (size_t n = 0; n < candidates->count(); ++n) {
            ret = UseCandidate(candidates->at(n));
            if (!ret)
                break;
        }
    }
    return ret;
}

bool KaerSession::UseCandidate(const IceCandidateInterface *candidate)
{
    size_t mediacontent_index = static_cast<size_t>(candidate->sdp_mline_index());
    size_t remote_content_size =
            BaseSession::remote_description()->contents().size();
    if (mediacontent_index >= remote_content_size) {
        LOG(LS_ERROR)
                << "UseRemoteCandidateInSession: Invalid candidate media index.";
        return false;
    }

    cricket::ContentInfo content =
            BaseSession::remote_description()->contents()[mediacontent_index];
    std::vector<cricket::Candidate> candidates;
    candidates.push_back(candidate->candidate());
    // Invoking BaseSession method to handle remote candidates.
    std::string error;
    if (OnRemoteCandidates(content.name, candidates, &error)) {
        // Candidates successfully submitted for checking.
        if (ice_connection_state_ == IceObserver::kIceConnectionNew ||
                ice_connection_state_ ==
                IceObserver::kIceConnectionDisconnected) {
            // If state is New, then the session has just gotten its first remote ICE
            // candidates, so go to Checking.
            // If state is Disconnected, the session is re-using old candidates or
            // receiving additional ones, so go to Checking.
            // If state is Connected, stay Connected.
            // TODO(bemasc): If state is Connected, and the new candidates are for a
            // newly added transport, then the state actually _should_ move to
            // checking.  Add a way to distinguish that case.
            SetIceConnectionState(IceObserver::kIceConnectionChecking);
        }
        // TODO(bemasc): If state is Completed, go back to Connected.
    } else {
        LOG(LS_WARNING) << error;
    }
    return true;
}

void KaerSession::SetIceConnectionState(IceObserver::IceConnectionState state)
{
    if (ice_connection_state_ == state) {
        return;
    }
    // ASSERT that the requested transition is allowed.  Note that
    // WebRtcSession does not implement "kIceConnectionClosed" (that is handled
    // within PeerConnection).  This switch statement should compile away when
    // ASSERTs are disabled.
    switch (ice_connection_state_) {
    case IceObserver::kIceConnectionNew:
        ASSERT(state == IceObserver::kIceConnectionChecking);
        break;
    case IceObserver::kIceConnectionChecking:
        ASSERT(state == IceObserver::kIceConnectionFailed ||
               state == IceObserver::kIceConnectionConnected);
        break;
    case IceObserver::kIceConnectionConnected:
        ASSERT(state == IceObserver::kIceConnectionDisconnected ||
               state == IceObserver::kIceConnectionChecking ||
               state == IceObserver::kIceConnectionCompleted);
        break;
    case IceObserver::kIceConnectionCompleted:
        ASSERT(state == IceObserver::kIceConnectionConnected ||
               state == IceObserver::kIceConnectionDisconnected);
        break;
    case IceObserver::kIceConnectionFailed:
        ASSERT(state == IceObserver::kIceConnectionNew);
        break;
    case IceObserver::kIceConnectionDisconnected:
        ASSERT(state == IceObserver::kIceConnectionChecking ||
               state == IceObserver::kIceConnectionConnected ||
               state == IceObserver::kIceConnectionCompleted ||
               state == IceObserver::kIceConnectionFailed);
        break;
    case IceObserver::kIceConnectionClosed:
        ASSERT(false);
        break;
    default:
        ASSERT(false);
        break;
    }

    ice_connection_state_ = state;
    if (ice_observer_) {
        ice_observer_->OnIceConnectionChange(ice_connection_state_);
    }
}

bool KaerSession::ValidateSessionDescription(
        const webrtc::SessionDescriptionInterface *sdesc,
        cricket::ContentSource source, std::string *error_desc)
{
    if (error() != cricket::BaseSession::ERROR_NONE) {
        return BadSdp(source, SessionErrorMsg(error()), error_desc);
    }

    if (!sdesc || !sdesc->description()) {
        return BadSdp(source, kInvalidSdp, error_desc);
    }

    std::string type = sdesc->type();
    Action action = GetAction(sdesc->type());
    if (source == cricket::CS_LOCAL) {
        if (!ExpectSetLocalDescription(action))
            return BadSdp(source, BadStateErrMsg(type, state()), error_desc);
    } else {
        if (!ExpectSetRemoteDescription(action))
            return BadSdp(source, BadStateErrMsg(type, state()), error_desc);
    }

    // Verify crypto settings.
    //    std::string crypto_error;
    //    if (webrtc_session_desc_factory_->Secure() == cricket::SEC_REQUIRED &&
    //        !VerifyCrypto(sdesc->description(), dtls_enabled_, &crypto_error)) {
    //      return BadSdp(source, crypto_error, error_desc);
    //    }
    // Verify ice-ufrag and ice-pwd.
    if (!VerifyIceUfragPwdPresent(sdesc->description())) {
        return BadSdp(source, kSdpWithoutIceUfragPwd, error_desc);
    }
    //        if (!ValidateBundleSettings(sdesc->description())) {
    //          return BadSdp(source, kBundleWithoutRtcpMux, error_desc);
    //        }

    // Verify m-lines in Answer when compared against Offer.
    if (action == kAnswer) {
        const cricket::SessionDescription* offer_desc =
                (source == cricket::CS_LOCAL) ? remote_desc_->description() :
                                                local_desc_->description();
        if (!VerifyMediaDescriptions(sdesc->description(), offer_desc)) {
            return BadSdp(source, kMlineMismatch, error_desc);
        }
    }
    return true;
}

bool KaerSession::ExpectSetLocalDescription(KaerSession::Action action)
{
    return ((action == kOffer && state() == STATE_INIT) ||
            // update local offer
            (action == kOffer && state() == STATE_SENTINITIATE) ||
            // update the current ongoing session.
            (action == kOffer && state() == STATE_RECEIVEDACCEPT) ||
            (action == kOffer && state() == STATE_SENTACCEPT) ||
            (action == kOffer && state() == STATE_INPROGRESS) ||
            // accept remote offer
            (action == kAnswer && state() == STATE_RECEIVEDINITIATE) ||
            (action == kAnswer && state() == STATE_SENTPRACCEPT) ||
            (action == kPrAnswer && state() == STATE_RECEIVEDINITIATE) ||
            (action == kPrAnswer && state() == STATE_SENTPRACCEPT));
}

bool KaerSession::ExpectSetRemoteDescription(KaerSession::Action action)
{
    return ((action == kOffer && state() == STATE_INIT) ||
            // update remote offer
            (action == kOffer && state() == STATE_RECEIVEDINITIATE) ||
            // update the current ongoing session
            (action == kOffer && state() == STATE_RECEIVEDACCEPT) ||
            (action == kOffer && state() == STATE_SENTACCEPT) ||
            (action == kOffer && state() == STATE_INPROGRESS) ||
            // accept local offer
            (action == kAnswer && state() == STATE_SENTINITIATE) ||
            (action == kAnswer && state() == STATE_RECEIVEDPRACCEPT) ||
            (action == kPrAnswer && state() == STATE_SENTINITIATE) ||
            (action == kPrAnswer && state() == STATE_RECEIVEDPRACCEPT));
}

std::string KaerSession::BadStateErrMsg(const std::string &type,
                                        cricket::BaseSession::State state)
{
    std::ostringstream desc;
    desc << "Called with type in wrong state, "
         << "type: " << type << " state: " << GetStateString(state);
    return desc.str();
}

bool KaerSession::CreateTransportProxies(const cricket::TransportInfos &tinfos)
{
    for (cricket::TransportInfos::const_iterator tinfo = tinfos.begin();
         tinfo != tinfos.end(); ++tinfo) {
        if (tinfo->description.transport_type != transport_type()) {
            return false;
        }
        GetOrCreateTransportProxy(tinfo->content_name);
    }
    return true;
}

cricket::TransportInfos KaerSession::GetEmptyTransportInfos(
        const cricket::ContentInfos &contents) const
{
    cricket::TransportInfos tinfos;
    for (cricket::ContentInfos::const_iterator content = contents.begin();
         content != contents.end(); ++content) {
        tinfos.push_back(cricket::TransportInfo(
                             content->name,
                             cricket::TransportDescription(transport_type(),
                                                           std::string(),
                                                           std::string())));
    }
    return tinfos;
}

void KaerSession::ProcessNewLocalCandidate(const std::string &content_name,
                                           const cricket::Candidates &candidates)
{
    int sdp_mline_index;
    if (!GetLocalCandidateMediaIndex(content_name, &sdp_mline_index)) {
        LOG(LS_ERROR) << "ProcessNewLocalCandidate: content name "
                      << content_name << " not found";
        return;
    }

    for (cricket::Candidates::const_iterator citer = candidates.begin();
         citer != candidates.end(); ++citer) {
        // Use content_name as the candidate media id.
        JsepIceCandidate candidate(content_name, sdp_mline_index, *citer);
        if (ice_observer_) {
            ice_observer_->OnIceCandidate(&candidate);
        }
        if (local_desc_) {
            local_desc_->AddCandidate(&candidate);
        }
    }
}

bool KaerSession::GetLocalCandidateMediaIndex(const std::string &content_name,
                                              int *sdp_mline_index)
{
    if (!BaseSession::local_description() || !sdp_mline_index)
        return false;

    bool content_found = false;
    const ContentInfos& contents = BaseSession::local_description()->contents();
    for (size_t index = 0; index < contents.size(); ++index) {
        if (contents[index].name == content_name) {
            *sdp_mline_index = static_cast<int>(index);
            content_found = true;
            break;
        }
    }
    return content_found;
}

void KaerSession::OnStreamChannelClosed(cricket::StreamChannelInterface *channel)
{
    LOG_T_F(INFO)<<"channel closed "<<channel->content_name();
    channel_ = NULL;
}

void KaerSession::OnTransportRequestSignaling(cricket::Transport *transport)
{
    ASSERT(signaling_thread()->IsCurrent());
    transport->OnSignalingReady();
    if (ice_observer_) {
        ice_observer_->OnIceGatheringChange(IceObserver::kIceGatheringGathering);
    }
}

void KaerSession::OnTransportConnecting(cricket::Transport *transport)
{
    ASSERT(signaling_thread()->IsCurrent());
    // start monitoring for the write state of the transport.
    OnTransportWritable(transport);
}

void KaerSession::OnTransportWritable(cricket::Transport *transport)
{
    ASSERT(signaling_thread()->IsCurrent());
    // TODO(bemasc): Expose more API from Transport to detect when
    // candidate selection starts or stops, due to success or failure.
    if (transport->all_channels_writable()) {
        if (ice_connection_state_ == IceObserver::kIceConnectionChecking ||
                ice_connection_state_ == IceObserver::kIceConnectionDisconnected)
        {
            SetIceConnectionState(IceObserver::kIceConnectionConnected);
        }
    } else if (transport->HasChannels()) {
        // If the current state is Connected or Completed, then there were writable
        // channels but now there are not, so the next state must be Disconnected.
        if (ice_connection_state_ == IceObserver::kIceConnectionConnected ||
                ice_connection_state_ == IceObserver::kIceConnectionCompleted)
        {
            SetIceConnectionState(IceObserver::kIceConnectionDisconnected);
        }
    }
}

void KaerSession::OnTransportProxyCandidatesReady(
        cricket::TransportProxy *proxy, const cricket::Candidates &candidates)
{
    ASSERT(signaling_thread()->IsCurrent());
    ProcessNewLocalCandidate(proxy->content_name(), candidates);
}

void KaerSession::OnCandidatesAllocationDone()
{
    ASSERT(signaling_thread()->IsCurrent());
    if (ice_observer_) {
        ice_observer_->OnIceGatheringChange(
                    IceObserver::kIceGatheringComplete);
        ice_observer_->OnIceComplete();
    }
}



}
