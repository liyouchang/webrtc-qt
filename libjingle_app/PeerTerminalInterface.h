#ifndef PEERTERMINALINTERFACE_H
#define PEERTERMINALINTERFACE_H

#include "talk/base/sigslot.h"
#include "talk/base/buffer.h"
#include <string>

class PeerTerminalInterface{
public:
    virtual int ConnectToPeer(const std::string &peer_id) = 0;
    virtual int CloseTunnel(const std::string &peer_id) = 0;
    virtual int SendByRouter(const std::string & peer_id,const std::string & data) =0;
    virtual int SendByTunnel(const std::string &peer_id,const std::string & data) =0;
    virtual int SendByTunnel(const std::string &peer_id,const char *data,size_t len) =0 ;
    sigslot::signal2<PeerTerminalInterface *,const std::string &> SignalTunnelOpened;
    sigslot::signal2<PeerTerminalInterface *,const std::string &> SignalTunnelClosed;

    sigslot::signal2<const std::string &,talk_base::Buffer &> SignalTunnelMessage;
    sigslot::signal2<const std::string &,const std::string &> SignalRouterMessage;


};
#endif // PEERTERMINALINTERFACE_H
