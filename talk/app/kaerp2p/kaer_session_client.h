#ifndef KAER_SESSION_CLIENT_H
#define KAER_SESSION_CLIENT_H

#include "talk/p2p/base/sessiondescription.h"
#include "talk/p2p/base/transportdescriptionfactory.h"
#include "talk/base/stream.h"
#include "talk/app/kaerp2p/KaerSession.h"
#include "talk/session/tunnel/tunnelsessionclient.h"
#include "talk/app/webrtc/jsep.h"
namespace kaerp2p{

class kaer_session_client:  public cricket::TunnelSessionClient
{
public:
    kaer_session_client(talk_base::Thread* worker=NULL);

    // MessageHandler interface
public:
    void OnMessage(talk_base::Message *msg);

    // TunnelSessionClientBase interface
protected:
    virtual  cricket::TunnelSession* CreateTunnelInternel(cricket::SessionDescription* offer);

protected:
    talk_base::BasicNetworkManager network_manager_;
    //talk_base::scoped_ptr<cricket::PortAllocator> allocator_;
    // TunnelSessionClientBase interface
};


}
#endif // KAER_SESSION_CLIENT_H
