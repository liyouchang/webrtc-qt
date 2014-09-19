#ifndef KETUNNELCAMERA_H
#define KETUNNELCAMERA_H

#include "KeMsgProcess.h"
#include "talk/base/json.h"

namespace kaerp2p{

class RecordReaderInterface;
class RecordReaderAvi;


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
    //virtual void SetPtz(std::string ptz_key,int param);
    virtual void GetCameraVideoInfo(int level,VideoInfo * info) = 0;

protected:
    static const int kVideoLevelNum = 3;
    sigslot::signal2<const char *, int> SignalVideoData1;
    sigslot::signal2<const char *, int> SignalVideoData2;
    sigslot::signal2<const char *, int> SignalVideoData3;
    sigslot::signal2<const char *, int> SignalAudioData;

    virtual void OnRecvVideoClarity(std::string peer_id, int clarity);
    virtual void OnRecvRecordQuery(std::string peer_id, std::string condition);
    virtual void OnToPlayFile(const std::string &peer_id,
                              const std::string &filename);
    virtual void OnRecvTalkData(const std::string & peer_id,
                                const char * data,int len);
    virtual void OnCommandJsonMsg(const Json::Value &jmessage,
                                  Json::Value * jresult);

    virtual Json::Value GetResultMsg(const std::string & command, bool result);
    virtual void ReportJsonMsg(const std::string &peerId,Json::Value &jmessage);
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

    int video_status;
    int audio_status;
    int talk_status;
protected:
    virtual void OnMessageRespond(talk_base::Buffer & msgData);
    virtual void RecvAskMediaReq(talk_base::Buffer &msgData);
    virtual void RecvPlayFile(talk_base::Buffer &msgData);
    virtual void RecvTalkData(talk_base::Buffer &msgData);
    virtual void RespPlayFileReq(int resp);
    virtual void RespAskMediaReq(const VideoInfo & info);
    virtual void ReportMediaStatus(int video,int audio,int talk);
protected:
    virtual void ConnectMedia(int video,int audio,int talk);
    virtual void OnRecordProcess(int percent);
private:
    RecordReaderAvi *recordReader;
};

}
#endif // KETUNNELCAMERA_H
