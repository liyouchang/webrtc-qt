#ifndef KEMSGPROCESSCONTAINER_H
#define KEMSGPROCESSCONTAINER_H

#include "peerterminal.h"
#include "KeMsgProcess.h"
class KeMsgProcessContainer:public sigslot::has_slots<>
{
public:
    KeMsgProcessContainer();
    enum{
        MSG_SENDFILEVIDEO
    };

    virtual void Initialize(PeerTerminalInterface *t);
    virtual void OnTunnelOpened(PeerTerminalInterface * t,const std::string & peer_id);
    virtual void OnTunnelClosed(PeerTerminalInterface * t,const std::string & peer_id);
    virtual void OnTunnelMessage(const std::string &peer_id, talk_base::Buffer &msg);
    virtual void OnRouterMessage(const std::string &peer_id,const std::string& msg);

    virtual KeMsgProcess * GetProcess(const std::string & peer_id);

    sigslot::signal2<const char *, int > SignalVideoData;
    sigslot::signal2<const char *, int > SignalAudioData;
protected:
    virtual void AddMsgProcess(KeMsgProcess * process);
    virtual void OnProcessNeedSend(const std::string & peer_id,const char * data,int len);


    std::vector<KeMsgProcess *> processes_;
    PeerTerminalInterface * terminal_;

};

#endif // KEMSGPROCESSCONTAINER_H
