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
        MSG_SEND_VIDEO,
        MSG_SEND_AUDIO,
        MSG_MEDIA_CONTROL
    };

    HisiMediaDevice();
    ~HisiMediaDevice();
    virtual bool Init(kaerp2p::PeerConnectionClientInterface *client);

    void OnMessage(talk_base::Message *msg);
    void SetVideoResolution(std::string r);
protected:
    //video == 0 start video ,audio == 0 start audio
    int GetVideoFrameType();
private:
    void SendVideoFrame(const char *data, int len);
    void SendAudioFrame(const char *data, int len);
    talk_base::Thread * media_thread_;
    int video_handle_;
    int audio_handle_;
    int video_frame_type_;
    char media_buffer_[MEDIA_BUFFER_LENGTH];


    // KeTunnelCamera interface
protected:
    void SetVideoClarity(int);
    void SetPtz(std::string ptz_key, int param);

    // KeMsgProcessContainer interface
};

#endif // HISIMEDIADEVICE_H
