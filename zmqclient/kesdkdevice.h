#ifndef KESDKDEVICE_H
#define KESDKDEVICE_H


#include "libjingle_app/ketunnelcamera.h"
#include "talk/base/messagehandler.h"

#include "zmq.h"
#include "zmq.hpp"
#include "zhelpers.hpp"
#include "zmsg.hpp"

#include "keapi/alarm_define.h"



class KeSdkDevice : public kaerp2p::KeTunnelCamera,public talk_base::MessageHandler
{
public:
    enum {
        MSG_MEDIA_CONTROL,
        MSG_NET_CHECK,
        MSG_CheckCloseStream,
        MSG_ZMQ_RECV
    };

    KeSdkDevice();
    virtual ~KeSdkDevice();
    bool Init(kaerp2p::PeerTerminalInterface *t);
    void GetCameraVideoInfo(int level, kaerp2p::VideoInfo *info);
    void OnTunnelOpened(kaerp2p::PeerTerminalInterface *t, const std::string &peer_id);

    virtual void OnCommandJsonMsg(const Json::Value &jmessage,
                                  Json::Value * jresult);

    virtual void OnMessage(talk_base::Message *msg);

    //emit when the net status changed
    sigslot::signal0<> SignalNetStatusChange;

    // open a level of stream ,the level is 1~4 ,
    // 1 for main video stream , 2 for sub video stream ,3 for extern video stream ,
    // 4 for audio stream
    void MediaStreamOpen(int level);

    //get a IDR frame , the IDR frame will get in 5 frames.
    void MediaGetIDR(int level);
    void SetNetInfo();

    std::string GetMacAddress();
    std::string GetTerminalType();
    //设置Ntp,zone is like +8:00
    void SetNtp(const std::string & ntpIp, int port, const std::string &zone);
    //接受对讲数据
    void OnRecvTalkData(const std::string &peer_id, const char *data, int len);

    //设备控制函数封装
protected:

    bool SetOsdTitle(const std::string & title);
    bool SetPtz(std::string control, int param);
    Json::Value GetWifiJsonArray();
    bool SetWifiInfo(Json::Value jparam);
    bool QueryRecord(Json::Value condition,Json::Value * jrecordList,int * totalNum);
    bool SetArmingStatus(int status);
    int GetArmingStatus();
    void QuitMainThread();

protected:
    //发送视频帧数据,level 1~3
    void SendVideoFrame(const char *data, int len,int level);
    //发送音频帧数据
    void SendAudioFrame(const char *data, int len);

    void MediaStreamOpen_d(int level);
    //检查stream是否正在被请求
    void CheckCloseStream_d();

    void CheckNetIp_d();
    //初始化视频信息
    void InitVideoInfo();

    void ZmqRepMsg_z();

    int video1_handle_;
    int video2_handle_;
    int video3_handle_;
    int audio_handle_;

    kaerp2p::VideoInfo video1_info_;
    kaerp2p::VideoInfo video2_info_;
    kaerp2p::VideoInfo video3_info_;

    talk_base::Thread *deviceThread;

    talk_base::Thread *zmqThread;

    zmq::context_t *zmqContext;
    zmq::socket_t *repSocket;

    int clock_handle;
    int ntp_chandle;
    int gpio_handle;
    int oldIp;

public:
    class RegisterCallBack{
    public:
        RegisterCallBack();
        static RegisterCallBack *Instance();
        sigslot::signal3<const char *, int ,int> SignalVideoFrame;
        sigslot::signal2<const char *, int > SignalAudioFrame;
        sigslot::signal0<> SignalReboot;
        sigslot::signal3<int,const std::string &,const std::string &> SignalTerminalAlarm;
        static int MainStreamCallBack(char * pFrameData,int iFrameLen);
        static int SubStreamCallBack(char * pFrameData,int iFrameLen);
        static int ExtStreamCallBack(char * pFrameData,int iFrameLen);
        static int AudioStreamCallBack(char * pFrameData,int iFrameLen);
        static int RebootCallback();
        static int AlarmCallback(st_alarm_upload_t *,char * pJpegData,int iJpegLen);
    };

};

class KeSdkProcess : public kaerp2p::KeMessageProcessCamera
{
public:
    KeSdkProcess(std::string peerId, KeSdkDevice * container);
    // KeMessageProcessCamera interface
protected:
    void ConnectMedia(int video, int audio, int talk);
};



//class SdkDeviceFunction : public talk_base::MessageHandler
//{
//public:
//    SdkDeviceFunction();

//    virtual ~SdkDeviceFunction();

//    class RegisterCallBack{
//    public:
//        RegisterCallBack();
//        static RegisterCallBack *Instance();
//        sigslot::signal3<const char *, int ,int> SignalVideoFrame;
//        sigslot::signal2<const char *, int > SignalAudioFrame;
//        sigslot::signal0<> SignalReboot;
//        static int MainStreamCallBack(char * pFrameData,int iFrameLen);
//        static int SubStreamCallBack(char * pFrameData,int iFrameLen);
//        static int ExtStreamCallBack(char * pFrameData,int iFrameLen);
//        static int AudioStreamCallBack(char * pFrameData,int iFrameLen);
//        static int RebootCallback();
//    };



//protected:
//    int video1_handle_;
//    int video2_handle_;
//    int video3_handle_;
//    int audio_handle_;


//    int clock_handle;
//    int gpio_handle;
//    int oldIp;

//    talk_base::Thread *mainThread;

//};


#endif // KESDKDEVICE_H
