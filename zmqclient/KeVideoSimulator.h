#ifndef KEVIDEOSIMULATOR_H
#define KEVIDEOSIMULATOR_H

#include "libjingle_app/KeMsgProcessContainer.h"
#include "talk/base/messagehandler.h"

namespace talk_base {
    class Thread;
    class Buffer;
}

class KeVideoSimulator:public talk_base::MessageHandler,public KeTunnelCamera
{
public:
    enum{
        MSG_SENDFILEVIDEO
    };
    KeVideoSimulator();
    ~KeVideoSimulator();

    bool ReadVideoData(std::string file_name);

    void SendMediaMsg(const char *data, int len);


protected:
    talk_base::Thread * media_thread_;
    talk_base::Buffer video_data_;
    std::vector<KeMessageProcessCamera *> processes_;
    std::string mac;
    // MessageHandler interface
public:
    void OnMessage(talk_base::Message *msg);
};

#endif // KEVIDEOSIMULATOR_H
