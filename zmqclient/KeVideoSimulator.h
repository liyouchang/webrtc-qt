#ifndef KEVIDEOSIMULATOR_H
#define KEVIDEOSIMULATOR_H

#include "libjingle_app/ketunnelcamera.h"
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


    // MessageHandler interface
public:
    void OnMessage(talk_base::Message *msg);

    // KeTunnelCamera interface
protected:
    void OnRecvRecordQuery(std::string peer_id, std::string condition);

protected:
    talk_base::Thread * media_thread_;
    talk_base::Buffer video_data_;

};

#endif // KEVIDEOSIMULATOR_H
