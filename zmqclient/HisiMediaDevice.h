/* HisiMediaDevice.h
 * HisiMediaDevice 用来对设备进行操作，封装了设备操作函数，实现KeTunnelCamera接口
 * copyright: Kaer inc. 2014
 * auth: lht
 *
 *
 */



#ifndef HISIMEDIADEVICE_H
#define HISIMEDIADEVICE_H
#include "libjingle_app/ketunnelcamera.h"
#include "talk/base/messagehandler.h"
namespace talk_base {
    class Thread;
    class Buffer;
}

#define MEDIA_BUFFER_LENGTH 512*1024

class KaerCameraProcess : public kaerp2p::KeMessageProcessCamera
{
public:
    KaerCameraProcess(std::string peerId, kaerp2p::KeTunnelCamera * container);
};


class HisiMediaDevice:talk_base::MessageHandler,public kaerp2p::KeTunnelCamera
{
public:
    enum{
        MSG_SEND_VIDEO,
        MSG_SEND_VIDEO_SUB,
        MSG_SEND_AUDIO,
        MSG_MEDIA_CONTROL,
        MSG_NET_CHECK
    };
    HisiMediaDevice();
    ~HisiMediaDevice();
    virtual bool Init(kaerp2p::PeerConnectionClientInterface *client);
    bool InitDeviceVideoInfo();
    virtual void SetVideoClarity(int);
    virtual void OnMessage(talk_base::Message *msg);
    void SetVideoResolution(std::string r);
    std::string GetHardwareId();
    virtual void GetCameraVideoInfo(int level,kaerp2p::VideoInfo * info);
    sigslot::signal0<> SignalNetStatusChange;


protected:
    int GetVideoFrameType(int level);
    int GetVideoFrameRate(int level);
    void CheckNetStatus();
private:
    /**
     * @brief SendVideoFrame
     * @param data
     * @param len
     * @param level : 1--video1,2--video2
     */
    void SendVideoFrame(const char *data, int len,int level);
    void SendAudioFrame(const char *data, int len);
    talk_base::Thread *media_thread_;
    int video1_handle_;
    int video2_handle_;
    int audio_handle_;
    kaerp2p::VideoInfo video1_info_;
    kaerp2p::VideoInfo video2_info_;
    int video_clarity_;
    char media_buffer_[MEDIA_BUFFER_LENGTH];
    int oldNetType;
    int oldIp;
    // KeTunnelCamera interface
protected:
    virtual int GetVideoClarity();
    virtual void SetPtz(std::string ptz_key, int param);
    virtual void OnRecvTalkData(const std::string &peer_id,
                                const char *data, int len);
    virtual void RecvGetWifiInfo(std::string peer_id);
    virtual void SetWifiInfo(std::string peerId, std::string param);
    virtual void OnRecvRecordQuery(std::string peer_id, std::string condition);
};

enum AlarmType{
    kAlarmMove = 1,
};

class AlarmNotify{
public:
    void StartNotify();
    sigslot::signal3<int,const std::string &,const std::string &> SignalTerminalAlarm;
    static AlarmNotify *Instance(){
        static AlarmNotify notify;
        return &notify;
    }
    static int NotifyCallBack(int chn,int rea,int io,
                              int snapcount,int snapsize,char* snapbuf);
};

#endif // HISIMEDIADEVICE_H
