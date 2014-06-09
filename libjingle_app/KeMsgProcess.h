
#ifndef KEMSGPROCESS_H
#define KEMSGPROCESS_H


#include <string>
#include <vector>

#include "talk/base/buffer.h"
#include "talk/base/sigslot.h"
#include "talk/base/messagehandler.h"

#include "PeerTerminalInterface.h"
#include "PeerConnectionClinetInterface.h"
#include "defaults.h"

namespace talk_base {
class Thread;
}

namespace kaerp2p{


class KeMsgProcessContainer;



struct RecordInfo{
    char fileName[80];
    int startTime;
    int endTime;
};

class KeMsgProcess:public talk_base::MessageHandler,public sigslot::has_slots<>
{
public:
    KeMsgProcess(std::string peer_id,KeMsgProcessContainer * container);
    virtual ~KeMsgProcess();

    enum {
        MSG_HEART_SENDED,
        MSG_HEART_RECEIVED
    };

    virtual void OnMessage(talk_base::Message *msg);
    virtual void StartHeartBeat();
    virtual void OnProcessMessage(const std::string & peer_id,talk_base::Buffer & msg);

    std::string peer_id(){ return peer_id_;}

    sigslot::signal1<const std::string &> SignalHeartStop;
    sigslot::signal3<const std::string &,const char * ,int> SignalNeedSendData;

    VideoInfo videoInfo_;

protected:
    virtual void ExtractMessage(talk_base::Buffer & allBytes);
    virtual void OnMessageRespond(talk_base::Buffer & msgData);
    virtual void SendHeart();

    KeMsgProcessContainer *container_;
    int heartSendDelay;
    int heartMissStop;
    int recvMsgMaxLen;

private:
    std::string peer_id_;
    //message extract param
    char head_buffer_[11];
    int buf_position_;
    int to_read_;
    talk_base::Buffer msg_received_;
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
    virtual bool OpenTunnel(const std::string &peer_id);
    virtual bool CloseTunnel(const std::string &peer_id);
    virtual bool IsTunnelOpened(const std::string &peer_id);
    virtual void OnTunnelOpened(PeerTerminalInterface * t,
                                const std::string & peer_id);
    virtual void OnTunnelClosed(PeerTerminalInterface * t,
                                const std::string & peer_id);
    virtual void OnTunnelMessage(const std::string &peer_id,
                                 talk_base::Buffer &msg);
    virtual void OnRouterMessage(const std::string &peer_id,
                                 talk_base::Buffer &msg);
    virtual void SendSelfData(const std::string & peer_id,
                              const char * data,int len);

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

}

#endif // KEMSGPROCESS_H
