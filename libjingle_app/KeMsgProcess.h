#ifndef KEMSGPROCESS_H
#define KEMSGPROCESS_H


#include <string>

#include "talk/base/buffer.h"
#include "talk/base/sigslot.h"
#include "talk/base/messagehandler.h"

namespace talk_base {
class Thread;
}

class KeMsgProcessContainer;
class KeTunnelCamera;
class KeTunnelClient;
class KeMsgProcess:public talk_base::MessageHandler,public sigslot::has_slots<>
{
public:
    enum {
        MSG_HEART_SENDED,
        MSG_HEART_RECEIVED
    };
    void OnMessage(talk_base::Message *msg);

    KeMsgProcess(std::string peer_id,KeMsgProcessContainer * container);
    sigslot::signal3<const std::string &,const char * ,int> SignalNeedSendData;
    void OnTunnelMessage(const std::string & peer_id,talk_base::Buffer & msg);
    std::string peer_id(){ return peer_id_;}

    void StartHeartBeat();
    sigslot::signal1<const std::string &> SignalHeartStop;
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


class KeMessageProcessCamera: public KeMsgProcess
{
public:
    KeMessageProcessCamera(std::string peer_id,KeTunnelCamera * container);
    void OnVideoData(const char *data, int len);
    void OnAudioData(const char * data,int len);
    void OnRecordData(const char * data,int len);

    sigslot::signal2<const std::string &,const std::string & > SignalToPlayFile;

protected:
    virtual void OnMessageRespond(talk_base::Buffer & msgData);
    virtual void RecvAskMediaMsg(talk_base::Buffer &msgData);
    virtual void RecvPlayFile(talk_base::Buffer &msgData);
private:
    bool video_started_;
    bool audio_started_;

};

class KeMessageProcessClient: public KeMsgProcess
{
public:
    KeMessageProcessClient(std::string peer_id,KeTunnelClient * container);

    void AskVideo(int vid,int video, int audio);
    void ReqestPlayFile(const char * file_name);

    sigslot::signal3<const std::string &,const char *,int > SignalRecvVideoData;
    sigslot::signal3<const std::string &,const char *,int > SignalRecvAudioData;

    sigslot::signal3<const std::string &,const char *,int > SignalRecvFileData;
    sigslot::signal2<const std::string &,int > SignalRecordPlayStatus;

protected:
    virtual void OnMessageRespond(talk_base::Buffer & msgData);
    virtual void OnRecvRecordMsg(talk_base::Buffer & msgData);
    virtual void OnRecvMediaData(talk_base::Buffer & msgData);
};



#endif // KEMSGPROCESS_H
