#ifndef KAERSESSIONDESCRIPTIONFACTORY_H
#define KAERSESSIONDESCRIPTIONFACTORY_H


#include "talk/base/messagehandler.h"
#include "talk/p2p/base/transportdescriptionfactory.h"
#include "talk/app/webrtc/jsep.h"
#include "talk/base/thread.h"
#include "kaer_session_client.h"

namespace kaerp2p {

class KaerSession;

class KaerSessionDescriptionFactory :
        public talk_base::MessageHandler,
        public sigslot::has_slots<>
{
public:
    KaerSessionDescriptionFactory(
        talk_base::Thread* signaling_thread,
        KaerSession* session,
        const std::string& session_id);

    virtual ~KaerSessionDescriptionFactory();

    static void CopyCandidatesFromSessionDescription(
            const webrtc::SessionDescriptionInterface* source_desc,
            webrtc::SessionDescriptionInterface* dest_desc);


    void CreateOffer(webrtc::CreateSessionDescriptionObserver* observer);
    void CreateAnswer(webrtc::CreateSessionDescriptionObserver* observer);
private:

    void PostCreateSessionDescriptionFailed(
            webrtc::CreateSessionDescriptionObserver* observer,
            const std::string& error);
    void PostCreateSessionDescriptionSucceeded(
            webrtc::CreateSessionDescriptionObserver* observer,
            webrtc::SessionDescriptionInterface* description);

    talk_base::Thread* signaling_thread_;
    //cricket::TransportDescriptionFactory transport_desc_factory_;
    uint64 session_version_;
    KaerSession* session_;
    std::string session_id_;
    kaer_session_client client_;

    // MessageHandler interface
public:
    void OnMessage(talk_base::Message *msg);
};
}
#endif // KAERSESSIONDESCRIPTIONFACTORY_H
