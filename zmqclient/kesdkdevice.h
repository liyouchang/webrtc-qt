#ifndef KESDKDEVICE_H
#define KESDKDEVICE_H


#include "libjingle_app/ketunnelcamera.h"
#include "talk/base/messagehandler.h"

class KeSdkDevice : public kaerp2p::KeTunnelCamera,public talk_base::MessageHandler
{
public:
    enum {
        MSG_MEDIA_CONTROL,
        MSG_NET_CHECK,
        MSG_CheckCloseStream,
    };

    KeSdkDevice();
    virtual ~KeSdkDevice();
    bool Init(kaerp2p::PeerTerminalInterface *t);
    void GetCameraVideoInfo(int level, kaerp2p::VideoInfo *info);
    void OnTunnelOpened(kaerp2p::PeerTerminalInterface *t, const std::string &peer_id);

    void OnCommandJsonMsg(const std::string &peerId, Json::Value &jmessage);

    virtual void OnMessage(talk_base::Message *msg);

    // open a level of stream ,the level is 1~4 ,
    // 1 for main video stream , 2 for sub video stream ,3 for extern video stream ,
    // 4 for audio stream
    void MediaStreamOpen(int level);

    //get a IDR frame , the IDR frame will get in 5 frames.
    void MediaGetIDR(int level);

    std::string GetMacAddress();
    //设置Ntp,zone is like +8:00
    void SetNtp(const std::string & ntpIp, int port, const std::string &zone);
    //接受对讲数据
    void OnRecvTalkData(const std::string &peer_id, const char *data, int len);

protected:
    //发送视频帧数据,level 1~3
    void SendVideoFrame(const char *data, int len,int level);
    //发送音频帧数据
    void SendAudioFrame(const char *data, int len);

    void MediaStreamOpen_d(int level);
    //检查stream是否正在被请求
    void CheckCloseStream_d();
    //初始化视频信息
    void InitVideoInfo();

    class RegisterCallBack{
    public:
        RegisterCallBack();
        static RegisterCallBack *Instance();
        sigslot::signal3<const char *, int ,int> SignalVideoFrame;
        sigslot::signal2<const char *, int > SignalAudioFrame;
        static int MainStreamCallBack(char * pFrameData,int iFrameLen);
        static int SubStreamCallBack(char * pFrameData,int iFrameLen);
        static int ExtStreamCallBack(char * pFrameData,int iFrameLen);
        static int AudioStreamCallBack(char * pFrameData,int iFrameLen);
    };

    int video1_handle_;
    int video2_handle_;
    int video3_handle_;
    int audio_handle_;

    kaerp2p::VideoInfo video1_info_;
    kaerp2p::VideoInfo video2_info_;
    kaerp2p::VideoInfo video3_info_;

    talk_base::Thread *deviceThread;

protected:
    bool SetOsdTitle(const std::string & title);

    bool SetPtz(std::string control, int param);
    Json::Value GetWifiJsonArray();
    bool SetWifiInfo(Json::Value jparam);
    virtual void OnRecvRecordQuery(std::string peer_id, std::string condition);

};

class KeSdkProcess : public kaerp2p::KeMessageProcessCamera
{
public:
    KeSdkProcess(std::string peerId, KeSdkDevice * container);
    // KeMessageProcessCamera interface
protected:
    void ConnectMedia(int video, int audio, int talk);
};



#endif // KESDKDEVICE_H
