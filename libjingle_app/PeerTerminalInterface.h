#ifndef PEERTERMINALINTERFACE_H
#define PEERTERMINALINTERFACE_H

#include "talk/base/sigslot.h"
#include "talk/base/buffer.h"
#include <string>

namespace kaerp2p{

class PeerTerminalInterface: public  sigslot::has_slots<>
{
public:
    virtual bool OpenTunnel(const std::string &peer_id) = 0;
    //tell teminal to close tunnel,after tunnel closed ,
    //you mast emit SignalTunnelClosed to verify the action
    //SignalTunnelClosed should not call by the same thread of CloseTunnel.
    virtual bool CloseTunnel(const std::string &peer_id) = 0;
    virtual bool SendByRouter(const std::string & peer_id,const std::string & data) =0;
    virtual bool SendByRouter(const std::string & peer_id,const char *data,size_t len) =0;
    virtual bool SendByTunnel(const std::string &peer_id,const std::string & data) =0;
    virtual bool SendByTunnel(const std::string &peer_id,const char *data,size_t len) =0 ;
    sigslot::signal2<PeerTerminalInterface *,const std::string &> SignalTunnelOpened;
    //emit when the tunnel closed by the other side or by call closetunnel function
    sigslot::signal2<PeerTerminalInterface *,const std::string &> SignalTunnelClosed;

    sigslot::signal2<const std::string &,talk_base::Buffer &> SignalTunnelMessage;
    sigslot::signal2<const std::string &,talk_base::Buffer &> SignalRouterMessage;


};
}

#endif // PEERTERMINALINTERFACE_H
