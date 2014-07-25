#ifndef KELOCALCLIENT_H
#define KELOCALCLIENT_H

#include "talk/base/asyncudpsocket.h"
#include "talk/base/thread.h"
#include "talk/base/scoped_ptr.h"
#include "talk/base/asynctcpsocket.h"
#include "talk/p2p/base/basicpacketsocketfactory.h"

#include "KeMsgProcess.h"
#include "KeMessage.h"
#include "ketunnelclient.h"

namespace kaerp2p{

class LocalTerminal: public  PeerTerminalInterface
{
    // PeerTerminalInterface interface
public:
    LocalTerminal();
    virtual ~LocalTerminal();
    bool Initialize();
    bool OpenTunnel(const std::string &peerAddr);
    bool CloseTunnel(const std::string &peerAddr);
    bool SendByRouter(const std::string &peerAddr, const std::string &data);
    bool SendByRouter(const std::string &peerAddr, const char *data, size_t len);
    bool SendByTunnel(const std::string &peerAddr, const std::string &data);
    bool SendByTunnel(const std::string &peerAddr, const char *data, size_t len);
protected:
    bool Initialize_s();
    bool OpenTunnel_s(const std::string &peerAddr);
    //    bool CloseTunnel_s(const std::string &peerAddr);
    //    bool SendByRouter_s(const std::string &peerAddr, const char *data, size_t len);
    //    bool SendByTunnel_s(const std::string &peerAddr, const char *data, size_t len);
    void OnLocalTcpConnect(talk_base::AsyncPacketSocket *socket );
    void OnLocalTcpClose(talk_base::AsyncPacketSocket* socket,int err);
    void OnPackage(talk_base::AsyncPacketSocket* socket, const char* buf, size_t size,
                   const talk_base::SocketAddress& remote_addr,
                   const talk_base::PacketTime& packet_time);
private:
    talk_base::AsyncUDPSocket * broadcastSocket;
    talk_base::AsyncPacketSocket * localSocket;
    talk_base::Thread* socketThread_;
    bool ownThread;
    std::string bindAddr;
    int socketType; //1:udp ,2 :tcp

    talk_base::scoped_ptr<talk_base::BasicPacketSocketFactory> socket_factory_;

};



class KeLocalMessage;
class KeLocalClient:public KeMsgProcessContainer
{
    friend class KeLocalMessage;
public:
    KeLocalClient();
public:
    virtual bool Init(PeerTerminalInterface *t);
    virtual bool StartPeerMedia(std::string peerAddr);
    virtual bool StopPeerMedia(std::string peerAddr);
    virtual void OnTunnelOpened(PeerTerminalInterface *t, const std::string &peer_id);
    virtual void OnRouterMessage(const std::string &peer_id, talk_base::Buffer &msg);
    bool SearchLocalDevice();

protected:
    virtual void OnSearchedDeviceInfo(const std::string &devInfo);

    virtual void OnRecvAudioData(const std::string & peer_id,
                                 const char * data,int len);
    virtual void OnRecvVideoData(const std::string & peer_id,
                                 const char * data,int len);
    talk_base::scoped_ptr<KeLocalMessage> broadcastMsg;
private:
    bool Init(PeerConnectionClientInterface *client);
};

class KeLocalMessage : public KeMsgProcess
{
public:
    KeLocalMessage(std::string peerAddr, KeLocalClient *container);
    virtual void AskMedia(int video, int listen, int talk);

    //json value returned
    sigslot::signal1<const std::string &> SignalNetDeviceInfo;
    sigslot::signal3<const std::string &,const char *,int > SignalRecvVideoData;
    sigslot::signal3<const std::string &,const char *,int > SignalRecvAudioData;

protected:
    void OnMessageRespond(talk_base::Buffer &msgData);
    virtual void OnRecvSearchOnlineDeviceResp(talk_base::Buffer &msgData);
    virtual void RecvVideoData(talk_base::Buffer & msgData);
    virtual void RecvAudioData(talk_base::Buffer & msgData);
    virtual void RecvAskMediaResp(talk_base::Buffer & msgData);
};

}
#endif // KELOCALCLIENT_H
