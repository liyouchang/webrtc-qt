#ifndef PEERTERMINAL_H
#define PEERTERMINAL_H


#include "peerconnectionclientdealer.h"
#include "../libjingle_app/p2pconductor.h"
#include "talk/app/webrtc/proxy.h"

//one terminal contains one PeerConnectionClient and some P2PConductor

class PeerTerminalInterface{
    virtual int Initialize(const std::string & router) = 0;
    virtual void ConnectToPeer(const std::string &peer_id) = 0;
    virtual void CloseTunnel() = 0;
    virtual int SendByRouter(const std::string & peer_id,const std::string & data) =0;
    virtual int SendByTunnel(const std::string & data) =0;
    virtual int SendByTunnel(const char *data,size_t len) =0 ;

};

class PeerTerminal:public PeerTerminalInterface,public sigslot::has_slots<>
{
public:
    PeerTerminal();
    virtual int Initialize(const std::string & router);
//    void CheckPeer();
    virtual void ConnectToPeer(const std::string &peer_id);
    virtual void CloseTunnel();
    virtual int SendByRouter(const std::string & peer_id,const std::string & data);
    virtual int SendByTunnel(const std::string & data);
    virtual int SendByTunnel(const char *data,size_t len);


protected:
    void OnTunnelOpened(kaerp2p::StreamProcess * tunnel);
    void OnTunnelReadData(kaerp2p::StreamProcess * tunnel,size_t len);
    kaerp2p::StreamProcess * tunnel_stream_;
    talk_base::scoped_ptr<PeerConnectionClientDealer> client_;
    talk_base::scoped_refptr<kaerp2p::P2PConductor> conductor_;

};

#endif // PEERTERMINAL_H
