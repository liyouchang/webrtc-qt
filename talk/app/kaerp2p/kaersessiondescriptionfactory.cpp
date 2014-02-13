#include "kaersessiondescriptionfactory.h"
#include "KaerSession.h"
#include "talk/app/webrtc/jsepsessiondescription.h"

namespace kaerp2p {

static const uint64 kInitSessionVersion = 2;

enum {
    MSG_CREATE_SESSIONDESCRIPTION_SUCCESS,
    MSG_CREATE_SESSIONDESCRIPTION_FAILED,
    MSG_GENERATE_IDENTITY,
};

struct CreateSessionDescriptionMsg : public talk_base::MessageData {
    explicit CreateSessionDescriptionMsg(
            webrtc::CreateSessionDescriptionObserver* observer)
        : observer(observer) {
    }

    talk_base::scoped_refptr<webrtc::CreateSessionDescriptionObserver> observer;
    std::string error;
    talk_base::scoped_ptr<webrtc::SessionDescriptionInterface> description;
};

KaerSessionDescriptionFactory::KaerSessionDescriptionFactory(
        talk_base::Thread *signaling_thread, KaerSession *session,
        const std::string &session_id):
    signaling_thread_(signaling_thread),
    session_version_(kInitSessionVersion),
    session_(session),
    session_id_(session_id)
{

}

KaerSessionDescriptionFactory::~KaerSessionDescriptionFactory()
{

}

void KaerSessionDescriptionFactory::CopyCandidatesFromSessionDescription(
        const webrtc::SessionDescriptionInterface *source_desc,
        webrtc::SessionDescriptionInterface *dest_desc)
{
    if (!source_desc)
        return;
    for (size_t m = 0; m < source_desc->number_of_mediasections() &&
         m < dest_desc->number_of_mediasections(); ++m) {
        const webrtc::IceCandidateCollection* source_candidates =
                source_desc->candidates(m);
        const webrtc::IceCandidateCollection* dest_candidates = dest_desc->candidates(m);
        for  (size_t n = 0; n < source_candidates->count(); ++n) {
            const webrtc::IceCandidateInterface* new_candidate = source_candidates->at(n);
            if (!dest_candidates->HasCandidate(new_candidate))
                dest_desc->AddCandidate(source_candidates->at(n));
        }
    }
}


void KaerSessionDescriptionFactory::CreateOffer(
        webrtc::CreateSessionDescriptionObserver *observer)
{
    cricket::SessionDescription* desc(
                client_.CreateOffer("test"));
    // RFC 3264
    // When issuing an offer that modifies the session,
    // the "o=" line of the new SDP MUST be identical to that in the
    // previous SDP, except that the version in the origin field MUST
    // increment by one from the previous SDP.

    // Just increase the version number by one each time when a new offer
    // is created regardless if it's identical to the previous one or not.
    // The |session_version_| is a uint64, the wrap around should not happen.
    ASSERT(session_version_ + 1 > session_version_);
    webrtc::JsepSessionDescription* offer(new webrtc::JsepSessionDescription(
                                      webrtc::JsepSessionDescription::kOffer));
    if (!offer->Initialize(desc, session_id_,
                           talk_base::ToString(session_version_++))) {
        delete offer;
        PostCreateSessionDescriptionFailed(observer, "CreateOffer failed.");
        return;
    }
    if (session_->local_desc_.get()) {
        // Include all local ice candidates in the SessionDescription unless
        // the an ice restart has been requested.
        CopyCandidatesFromSessionDescription(session_->local_desc_.get(), offer);
    }
    PostCreateSessionDescriptionSucceeded(observer, offer);
}

void KaerSessionDescriptionFactory::CreateAnswer(webrtc::CreateSessionDescriptionObserver *observer)
{
    cricket::SessionDescription* desc(client_.CreateAnswer(session_->remote_description()));
    // RFC 3264
    // If the answer is different from the offer in any way (different IP
    // addresses, ports, etc.), the origin line MUST be different in the answer.
    // In that case, the version number in the "o=" line of the answer is
    // unrelated to the version number in the o line of the offer.
    // Get a new version number by increasing the |session_version_answer_|.
    // The |session_version_| is a uint64, the wrap around should not happen.
    ASSERT(session_version_ + 1 > session_version_);
    webrtc::JsepSessionDescription* answer(new webrtc::JsepSessionDescription(
                                       webrtc::JsepSessionDescription::kAnswer));
    if (!answer->Initialize(desc, session_id_,
                            talk_base::ToString(session_version_++))) {
        delete answer;
        PostCreateSessionDescriptionFailed(observer,
                                           "CreateAnswer failed.");
        return;
    }
    if (session_->local_desc_.get()) {
        // Include all local ice candidates in the SessionDescription unless
        // the remote peer has requested an ice restart.
        CopyCandidatesFromSessionDescription(session_->local_desc_.get(), answer);
    }
    session_->ResetIceRestartLatch();
    PostCreateSessionDescriptionSucceeded(observer, answer);
}

void KaerSessionDescriptionFactory::PostCreateSessionDescriptionFailed(webrtc::CreateSessionDescriptionObserver *observer, const std::string &error)
{
    CreateSessionDescriptionMsg* msg = new CreateSessionDescriptionMsg(observer);
    msg->error = error;
    signaling_thread_->Post(this, MSG_CREATE_SESSIONDESCRIPTION_FAILED, msg);

}

void KaerSessionDescriptionFactory::PostCreateSessionDescriptionSucceeded(webrtc::CreateSessionDescriptionObserver *observer, webrtc::SessionDescriptionInterface *description)
{
    CreateSessionDescriptionMsg* msg = new CreateSessionDescriptionMsg(observer);
    msg->description.reset(description);
    signaling_thread_->Post(this, MSG_CREATE_SESSIONDESCRIPTION_SUCCESS, msg);

}

void KaerSessionDescriptionFactory::OnMessage(talk_base::Message *msg)
{
    switch (msg->message_id) {
    case MSG_CREATE_SESSIONDESCRIPTION_SUCCESS: {
        CreateSessionDescriptionMsg* param =
                static_cast<CreateSessionDescriptionMsg*>(msg->pdata);
        param->observer->OnSuccess(param->description.release());
        delete param;
        break;
    }
    case MSG_CREATE_SESSIONDESCRIPTION_FAILED: {
        CreateSessionDescriptionMsg* param =
                static_cast<CreateSessionDescriptionMsg*>(msg->pdata);
        param->observer->OnFailure(param->error);
        delete param;
        break;
    }
    case MSG_GENERATE_IDENTITY: {
        //LOG(LS_INFO) << "Generating identity.";
       // SetIdentity(talk_base::SSLIdentity::Generate(kWebRTCIdentityName));
        break;
    }
    default:
        ASSERT(false);
        break;
    }
}

}
