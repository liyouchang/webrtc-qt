#ifndef PEERTERMINAL_H
#define PEERTERMINAL_H


#include "p2pconductor.h"
#include "PeerTerminalInterface.h"
#include "PeerConnectionClinetInterface.h"
#include <vector>
#include "talk/p2p/base/basicpacketsocketfactory.h"
#include "talk/base/asyncudpsocket.h"

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
    PeerTerminal(kaerp2p::PeerConnectionClientInterface * client,
                 int max_tunnel = kInfiniteTunnel);

    virtual bool OpenTunnel(const std::string &peer_id);
    virtual bool CloseTunnel(const std::string &peer_id);
    //virtual bool TunnelOpened();
    virtual bool SendByRouter(const std::string & peer_id,const std::string & data);
    virtual bool SendByRouter(const std::string & peer_id,const char *data,size_t len);
    virtual bool SendByTunnel(const std::string &peer_id,const std::string & data);
    virtual bool SendByTunnel(const std::string &peer_id,const char *data,size_t len);

    //int AskTunnelVideo();
protected:
    void OnTunnelOpened(const std::string &peerId);
    void OnTunnelClosed(const std::string &peerId);
    void OnTunnelReadData(kaerp2p::StreamProcess * stream, talk_base::Buffer &buffer);
    void OnRouterReadData(const std::string & peer_id, const std::string & msg);
    void OnTunnelNeedSend(const std::string& peer_id, const std::string& msg);

private:
    ScopedTunnel GetTunnel(const std::string & peer_id);
    ScopedTunnel GetTunnel(kaerp2p::StreamProcess * stream);
    int CountAvailableTunnels();
    ScopedTunnel GetOrCreateTunnel(const std::string & peer_id);

    void SendTunnelError(const std::string &peer_id);
    kaerp2p::PeerConnectionClientInterface * client_;
    //talk_base::scoped_refptr<kaerp2p::P2PConductor> conductor_;
    std::vector<ScopedTunnel> tunnels_;
    talk_base::CriticalSection crit_;

    int max_tunnel_num_;

};

class LocalUdpTerminal: public  PeerTerminalInterface
{
    // PeerTerminalInterface interface
public:
    LocalUdpTerminal();
    virtual ~LocalUdpTerminal();
    bool Initialize(const std::string & localAddr);
    bool OpenTunnel(const std::string &peerAddr);
    bool CloseTunnel(const std::string &peerAddr);
    bool SendByRouter(const std::string &peerAddr, const std::string &data);
    bool SendByRouter(const std::string &peerAddr, const char *data, size_t len);
    bool SendByTunnel(const std::string &peerAddr, const std::string &data);
    bool SendByTunnel(const std::string &peerAddr, const char *data, size_t len);
protected:
    bool Initialize_s(const std::string & localAddr);
    //bool OpenTunnel_s(const std::string &peerAddr);
    bool SendByTunnel_s(const std::string &peerAddr, const char *data, size_t len);
    void OnPackage(talk_base::AsyncPacketSocket* socket,
                   const char* buf, size_t size,
                   const talk_base::SocketAddress& remote_addr,
                   const talk_base::PacketTime& packet_time);
private:
    talk_base::AsyncPacketSocket * localSocket;
    talk_base::Thread* socketThread_;
    bool ownThread;
    std::string bindAddr;
    bool isOpened;
    talk_base::scoped_ptr<talk_base::BasicPacketSocketFactory> socket_factory_;

};




}
#endif // PEERTERMINAL_H
