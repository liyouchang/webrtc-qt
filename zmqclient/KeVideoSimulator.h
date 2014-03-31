#ifndef KEVIDEOSIMULATOR_H
#define KEVIDEOSIMULATOR_H

#include "talk/base/thread.h"
#include "talk/base/fileutils.h"
#include "talk/base/buffer.h"
#include "../libjingle_app/KeMsgProcessContainer.h"

class KeVideoSimulator:public talk_base::MessageHandler,public KeMsgProcessContainer
{
public:
    enum{
        MSG_SENDFILEVIDEO
    };
    KeVideoSimulator();
    ~KeVideoSimulator();

    bool ReadVideoData(std::string file_name);
    void OnTunnelOpened(PeerTerminalInterface * t,const std::string & peer_id);

    void SendMediaMsg(const char *data, int len);

    sigslot::signal2<const char *, int > SignalVideoData;
    sigslot::signal2<const char *, int > SignalAudioData;

protected:
    void OnProcessMediaRequest(KeMessageProcessCamera * process,int video,int audio);

    talk_base::Thread * media_thread_;
    talk_base::Buffer video_data_;
    std::vector<KeMessageProcessCamera *> processes_;
    //bool startSend;

    PeerTerminalInterface * terminal_;
    // MessageHandler interface
public:
    void OnMessage(talk_base::Message *msg);
};

#endif // KEVIDEOSIMULATOR_H
