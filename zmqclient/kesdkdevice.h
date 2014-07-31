#ifndef KESDKDEVICE_H
#define KESDKDEVICE_H


#include "libjingle_app/ketunnelcamera.h"



class KeSdkDevice : public kaerp2p::KeTunnelCamera
{
public:
    KeSdkDevice();
    virtual ~KeSdkDevice();
    bool Init(kaerp2p::PeerTerminalInterface *t);
    void GetCameraVideoInfo(int level, kaerp2p::VideoInfo *info);
    void OnTunnelOpened(kaerp2p::PeerTerminalInterface *t, const std::string &peer_id);
    void OnRecvTalkData(const std::string &peer_id, const char *data, int len);
    void OnCommandJsonMsg(const std::string &peerId, Json::Value &jmessage);

protected:
    void SendVideoFrame(const char *data, int len,int level);
    void SendAudioFrame(const char *data, int len);

    class RegisterCallBack{
    public:
        RegisterCallBack();
        static RegisterCallBack *Instance();
        sigslot::signal3<const char *, int ,int> SignalVideoFrame;
        sigslot::signal2<const char *, int > SignalAudioFrame;
        static int FirstStreamCallBack(char * pFrameData,int iFrameLen);

    };

    kaerp2p::VideoInfo video1_info_;
    kaerp2p::VideoInfo video2_info_;
    kaerp2p::VideoInfo video3_info_;
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
