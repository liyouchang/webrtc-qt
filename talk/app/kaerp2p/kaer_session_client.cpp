#include "kaer_session_client.h"
#include "talk/p2p/base/transportdescription.h"
#include "talk/p2p/base/transportinfo.h"
#include "talk/p2p/base/constants.h"
#include "talk/p2p/client/basicportallocator.h"

namespace kaerp2p{


kaer_session_client::kaer_session_client(talk_base::Thread *worker):
    TunnelSessionClient()
{
    signaling_thread_ = talk_base::Thread::Current();
    if (worker == NULL) {
      worker_thread_ = talk_base::Thread::Current();
    } else {
      worker_thread_ = worker;
    }

//    allocator_.reset(new cricket::BasicPortAllocator(
//                   &network_manager_, stun_addr, talk_base::SocketAddress(),
//                   talk_base::SocketAddress(), talk_base::SocketAddress()));
}

void kaer_session_client::OnMessage(talk_base::Message *msg)
{
    cricket::TunnelSessionClient::OnMessage(msg);
}

cricket::TunnelSession *kaer_session_client::CreateTunnelInternel(cricket::SessionDescription *offer)
{
//    KaerSession* session = new KaerSession(signaling_thread(),
//                                           worker_thread(),
 //                                          allocator_.get(),namespace_);

//    cricket::TunnelSession* tunnel = MakeTunnelSession(session, signaling_thread(),
//                                                    cricket::INITIATOR);
//    sessions_.push_back(tunnel);
//    //session->Initiate(this,offer);
//    return tunnel;
    return NULL;
}


}
