#ifndef HISIMEDIADEVICE_H
#define HISIMEDIADEVICE_H


#include "talk/base/buffer.h"
#include "peerterminal.h"
#include "talk/base/thread.h"
#include "KeMessageProcessCamera.h"

#define MEDIA_BUFFER_LENGTH 512*1024
class HisiMediaDevice:talk_base::MessageHandler,public sigslot::has_slots<>
{
public:
    enum{
        MSG_SEND_VIDEO,
        MSG_MEDIA_CONTROL
    };

    HisiMediaDevice();
    ~HisiMediaDevice();
    void OnTunnelOpend(PeerTerminalInterface * t,const std::string & peer_id);
    //video == 0 start video ,audio == 0 start audio
    void OnMediaRequest(int video,int audio);

private:
    int MediaControl_m(int video,int audio);
    talk_base::Thread * media_thread_;

    KeMessageProcessCamera * process_;
    int video_handle_;
    char media_buffer_[MEDIA_BUFFER_LENGTH];
    // MessageHandler interface
public:
    void OnMessage(talk_base::Message *msg);
};

#endif // HISIMEDIADEVICE_H
