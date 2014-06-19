#ifndef KETUNNELCAMERA_H
#define KETUNNELCAMERA_H

#include "KeMsgProcess.h"
#include "talk/base/json.h"

namespace kaerp2p{

class RecordReaderInterface;

class KeTunnelCamera:public KeMsgProcessContainer
{
    friend class KeMessageProcessCamera;
public:
    virtual void OnTunnelOpened(PeerTerminalInterface * t,
                                const std::string & peer_id);
    virtual void OnRouterMessage(const std::string &peer_id,
                                 talk_base::Buffer &msg);
    virtual void SetVideoClarity(int);
    virtual int GetVideoClarity();
    virtual void SetPtz(std::string  ptz_key,int param);
    virtual void GetCameraVideoInfo(int level,VideoInfo * info) = 0;

protected:
    sigslot::signal2<const char *, int> SignalVideoData1;
    sigslot::signal2<const char *, int> SignalVideoData2;
    sigslot::signal2<const char *, int > SignalAudioData;
    virtual void OnRecvVideoClarity(std::string peer_id,int clarity);
    virtual void OnRecvRecordQuery(std::string peer_id, std::string condition);
    virtual void RecvGetWifiInfo(std::string peer_id);
    virtual void SetWifiInfo(std::string peer_id,std::string param);
    virtual void OnToPlayFile(const std::string &peer_id,
                              const std::string &filename);
    virtual void OnRecvTalkData(const std::string & peer_id,
                                const char * data,int len);
    virtual void OnCommandJsonMsg(const std::string &peerId, Json::Value &jmessage);
};


class KeMessageProcessCamera: public KeMsgProcess
{
public:
    KeMessageProcessCamera(std::string peer_id,KeTunnelCamera * container);
    virtual ~KeMessageProcessCamera();
    void OnVideoData(const char *data, int len);
    void OnAudioData(const char * data,int len);
    void OnRecordData(const char * data,int len);
    void OnRecordReadEnd(RecordReaderInterface *reader);
    sigslot::signal3<const std::string &,const char *,int > SignalRecvTalkData;
protected:
    virtual void OnMessageRespond(talk_base::Buffer & msgData);
    virtual void RecvAskMediaReq(talk_base::Buffer &msgData);
    virtual void RecvPlayFile(talk_base::Buffer &msgData);
    virtual void RecvTalkData(talk_base::Buffer &msgData);
protected:
    virtual void RespAskMediaReq(const VideoInfo & info);
    virtual void RespPlayFileReq(int resp, const char *fileName);
    virtual void ConnectMedia(int video,int audio,int talk);
private:
    bool video_started_;
    bool audio_started_;
    bool talk_started_;
    RecordReaderInterface *recordReader;

};

}
#endif // KETUNNELCAMERA_H
