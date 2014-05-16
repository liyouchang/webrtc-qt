
#ifndef KEMSGPROCESS_H
#define KEMSGPROCESS_H


#include <string>
#include <vector>

#include "talk/base/buffer.h"
#include "talk/base/sigslot.h"
#include "talk/base/messagehandler.h"

#include "PeerTerminalInterface.h"
#include "PeerConnectionClinetInterface.h"

namespace talk_base {
class Thread;
}

class KeMsgProcessContainer;

struct VideoInfo{
    int frameRate_;
    int frameType_;
};

class KeMsgProcess:public talk_base::MessageHandler,public sigslot::has_slots<>
{
public:
    enum {
        MSG_HEART_SENDED,
        MSG_HEART_RECEIVED
    };
    void OnMessage(talk_base::Message *msg);

    KeMsgProcess(std::string peer_id,KeMsgProcessContainer * container);
    virtual ~KeMsgProcess();
    sigslot::signal3<const std::string &,const char * ,int> SignalNeedSendData;
    void OnTunnelMessage(const std::string & peer_id,talk_base::Buffer & msg);
    std::string peer_id(){ return peer_id_;}

    void StartHeartBeat();
    sigslot::signal1<const std::string &> SignalHeartStop;

    VideoInfo videoInfo_;

protected:
    virtual void ExtractMessage(talk_base::Buffer & allBytes);
    virtual void OnMessageRespond(talk_base::Buffer & msgData);
    void SendHeart();

    KeMsgProcessContainer *container_;

private:
    std::string peer_id_;
    //PeerTerminalInterface * terminal_;
    //message extract param
    char head_buffer_[11];
    int buf_position_;
    int to_read_;
    talk_base::Buffer msg_received_;
    const int kMsgMaxLen;
    //heart beat param
    talk_base::Thread * heart_thread_;
    int heart_count_;
};


class KeMsgProcessContainer: public sigslot::has_slots<>
{
    friend class KeMsgProcess;
public:
    KeMsgProcessContainer();
    virtual ~KeMsgProcessContainer();

    virtual bool Init(PeerTerminalInterface *t);
    virtual bool Init(kaerp2p::PeerConnectionClientInterface * client);
    virtual int OpenTunnel(const std::string &peer_id);
    virtual int CloseTunnel(const std::string &peer_id);
    virtual bool IsTunnelOpened(const std::string &peer_id);
    virtual void OnTunnelOpened(PeerTerminalInterface * t,
                                const std::string & peer_id);
    virtual void OnTunnelClosed(PeerTerminalInterface * t,
                                const std::string & peer_id);
    virtual void OnTunnelMessage(const std::string &peer_id,
                                 talk_base::Buffer &msg);
    virtual void OnRouterMessage(const std::string &peer_id,
                                 const std::string& msg);

protected:
    virtual KeMsgProcess * GetProcess(const std::string & peer_id);
    virtual void AddMsgProcess(KeMsgProcess * process);
    virtual void SendProcessData(const std::string & peer_id,
                                   const char * data,int len);
    virtual void OnHeartStop(const std::string & peer_id);

    std::vector<KeMsgProcess *> processes_;
    PeerTerminalInterface * terminal_;
    bool has_terminal;
};



#endif // KEMSGPROCESS_H
