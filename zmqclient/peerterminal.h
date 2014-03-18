#ifndef PEERTERMINAL_H
#define PEERTERMINAL_H


#include "peerconnectionclientdealer.h"
#include "p2pconductor.h"

//one terminal contains one PeerConnectionClient and some P2PConductor

class PeerTerminal:public sigslot::has_slots<>
{
public:
    PeerTerminal();
    virtual int Initialize(const std::string & router);
//    void CheckPeer();
    virtual void ConnectToPeer(const std::string &peer_id);
    virtual void CloseTunnel();
    int SendByRouter(const std::string & peer_id,const std::string & data);
    int SendByTunnel(const std::string & data);
    int SendByTunnel(const char *data,size_t len);


protected:
    void OnTunnelOpened(kaerp2p::StreamProcess * tunnel);
    void OnTunnelReadData(kaerp2p::StreamProcess * tunnel,size_t len);
    kaerp2p::StreamProcess * tunnelStream_;
    talk_base::scoped_ptr<PeerConnectionClientDealer> client_;
    talk_base::scoped_refptr<kaerp2p::P2PConductor> conductor_;

};

#endif // PEERTERMINAL_H
