#ifndef KEMSGPROCESS_H
#define KEMSGPROCESS_H


#include <string>

#include "talk/base/buffer.h"
#include "talk/base/sigslot.h"
#include "talk/base/messagehandler.h"

namespace talk_base {
class Thread;
}

class KeMsgProcess:public talk_base::MessageHandler,public sigslot::has_slots<>
{
public:

    KeMsgProcess(std::string peer_id);
    sigslot::signal3<const std::string &,const char * ,int> SignalNeedSendData;
    void OnTunnelMessage(const std::string & peer_id,talk_base::Buffer & msg);
    std::string GetPeerID(){ return peer_id_;}

    void StartHeartBeat();
    sigslot::signal1<const std::string &> SignalHeartStop;
protected:
    virtual void ExtractMessage(talk_base::Buffer & allBytes);
    virtual void OnMessageRespond(talk_base::Buffer & msgData);
    void SendHeart();

    std::string peer_id_;
    //PeerTerminalInterface * terminal_;
    //message extract param
    char head_buffer_[11];
    int buf_position_;
    int to_read_;
    talk_base::Buffer msg_received_;
    const int kMsgMaxLen;
    //heart beat param
    talk_base::Thread * msg_thread_;
    int heart_count_;
public:
    enum {
        MSG_HEART_SENDED,
        MSG_HEART_RECEIVED
    };
    void OnMessage(talk_base::Message *msg);

};


class KeMessageProcessCamera: public KeMsgProcess
{
public:
    KeMessageProcessCamera(std::string peer_id);
    sigslot::signal3<KeMessageProcessCamera *,int,int> SignalRecvAskMediaMsg;
    void OnVideoData(const char *data, int len);
    void OnAudioData(const char * data,int len);
protected:
    virtual void OnMessageRespond(talk_base::Buffer & msgData);
    void RecvAskMediaMsg(talk_base::Buffer &msgData);

    bool start_video_;
};

class KeMessageProcessClient: public KeMsgProcess
{
public:
    KeMessageProcessClient(std::string peer_id);

    void AskVideo();

    sigslot::signal3<const std::string &,const char *,int > SignalRecvVideoData;
    sigslot::signal3<const std::string &,const char *,int > SignalRecvAudioData;
protected:
    virtual void OnMessageRespond(talk_base::Buffer & msgData);

};



#endif // KEMSGPROCESS_H
