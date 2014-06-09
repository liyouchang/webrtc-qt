#ifndef PEERTERMINAL_H
#define PEERTERMINAL_H


#include "p2pconductor.h"
#include "PeerTerminalInterface.h"
#include "PeerConnectionClinetInterface.h"
#include <vector>
//one terminal contains one PeerConnectionClient and some P2PConductor
namespace kaerp2p{


const int kInfiniteTunnel = -1;


typedef talk_base::scoped_refptr<kaerp2p::P2PConductor> ScopedTunnel;

//this is a abstraction for communication of peer to peer
//implate a terminal that use P2PConductor to communicate with another terminal by p2p connection
//use a kaerp2p::PeerConnectionClientInterface to commucate with  another terminal from server
class PeerTerminal:public PeerTerminalInterface
{
public:
    PeerTerminal();
    //need a client that has been login
    int Initialize(kaerp2p::PeerConnectionClientInterface * client,int max_tunnel = kInfiniteTunnel);

    virtual bool OpenTunnel(const std::string &peer_id);
    virtual bool CloseTunnel(const std::string &peer_id);
    //virtual bool TunnelOpened();
    virtual bool SendByRouter(const std::string & peer_id,const std::string & data);
    virtual bool SendByRouter(const std::string & peer_id,const char *data,size_t len);
    virtual bool SendByTunnel(const std::string &peer_id,const std::string & data);
    virtual bool SendByTunnel(const std::string &peer_id,const char *data,size_t len);

    //int AskTunnelVideo();
protected:
    void OnTunnelOpened(kaerp2p::StreamProcess * stream);
    void OnTunnelClosed(kaerp2p::StreamProcess * tunnel);
    void OnTunnelReadData(kaerp2p::StreamProcess * stream,size_t len);
    void OnRouterReadData(const std::string & peer_id, const std::string & msg);
    void OnTunnelNeedSend(const std::string& peer_id, const std::string& msg);
private:
    ScopedTunnel GetTunnel(const std::string & peer_id);
    ScopedTunnel GetTunnel(kaerp2p::StreamProcess * stream);
    int CountAvailableTunnels();
    ScopedTunnel GetOrCreateTunnel(const std::string & peer_id);

    kaerp2p::PeerConnectionClientInterface * client_;
    //talk_base::scoped_refptr<kaerp2p::P2PConductor> conductor_;
    std::vector<ScopedTunnel> tunnels_;
    int max_tunnel_num_;

};
}
#endif // PEERTERMINAL_H
