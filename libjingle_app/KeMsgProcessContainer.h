#ifndef KEMSGPROCESSCONTAINER_H
#define KEMSGPROCESSCONTAINER_H

#include "PeerTerminalInterface.h"
#include "PeerConnectionClinetInterface.h"
#include <vector>

class KeMsgProcess;
class KeMessageProcessCamera;

class KeMsgProcessContainer:public sigslot::has_slots<>
{
public:
    KeMsgProcessContainer();
    virtual ~KeMsgProcessContainer();

    virtual int Initialize(PeerTerminalInterface *t);
    virtual int Initialize(kaerp2p::PeerConnectionClientInterface * client);
    virtual int OpenTunnel(const std::string &peer_id);
    virtual int CloseTunnel(const std::string &peer_id);

    virtual void OnTunnelOpened(PeerTerminalInterface * t,const std::string & peer_id);
    virtual void OnTunnelClosed(PeerTerminalInterface * t,const std::string & peer_id);
    virtual void OnTunnelMessage(const std::string &peer_id, talk_base::Buffer &msg);
    virtual void OnRouterMessage(const std::string &peer_id,const std::string& msg);

protected:
    virtual KeMsgProcess * GetProcess(const std::string & peer_id);
    virtual void AddMsgProcess(KeMsgProcess * process);
    virtual void OnProcessNeedSend(const std::string & peer_id,const char * data,int len);


    std::vector<KeMsgProcess *> processes_;
    PeerTerminalInterface * terminal_;
    bool has_terminal;
};

class KeTunnelClient:public KeMsgProcessContainer{
public:
    KeTunnelClient();
    virtual int AskPeerVideo(std::string peer_id);
    virtual void OnTunnelOpened(PeerTerminalInterface * t,const std::string & peer_id);
protected:
    virtual void OnRecvAudioData(const std::string & peer_id,const char * data,int len);
    virtual void OnRecvVideoData(const std::string & peer_id,const char * data,int len);
};

class KeTunnelCamera:public KeMsgProcessContainer{
public:
    virtual void OnTunnelOpened(PeerTerminalInterface * t,const std::string & peer_id);
protected:
    virtual void OnProcessMediaRequest(KeMessageProcessCamera * process,int video,int audio);
    sigslot::signal2<const char *, int > SignalVideoData;
    sigslot::signal2<const char *, int > SignalAudioData;

};

#endif // KEMSGPROCESSCONTAINER_H
