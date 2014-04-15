#ifndef HISIMEDIADEVICE_H
#define HISIMEDIADEVICE_H


#include "libjingle_app/KeMsgProcessContainer.h"
#include "talk/base/messagehandler.h"
namespace talk_base {
    class Thread;
    class Buffer;
}



#define MEDIA_BUFFER_LENGTH 512*1024

class HisiMediaDevice:talk_base::MessageHandler,public KeTunnelCamera
{
public:
    enum{
        MSG_START_VIDEO,
        MSG_STOP_VIDEO,
        MSG_SEND_VIDEO,
        MSG_MEDIA_CONTROL
    };

    HisiMediaDevice();
    ~HisiMediaDevice();
    virtual void OnTunnelClosed(PeerTerminalInterface * t,const std::string & peer_id);
    void OnMessage(talk_base::Message *msg);

protected:
    //video == 0 start video ,audio == 0 start audio
    void OnProcessMediaRequest(KeMessageProcessCamera *process, int video, int audio);


private:
    void SendVideoFrame(const char *data, int len);
    talk_base::Thread * media_thread_;
    int video_handle_;
    char media_buffer_[MEDIA_BUFFER_LENGTH];

};

#endif // HISIMEDIADEVICE_H
