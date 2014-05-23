#ifndef KEVIDEOSIMULATOR_H
#define KEVIDEOSIMULATOR_H

#include "libjingle_app/ketunnelcamera.h"
#include "talk/base/messagehandler.h"
#include "libjingle_app/recorderavi.h"

namespace talk_base {
    class Thread;
    class Buffer;
}

class KeVideoSimulator:public kaerp2p::KeTunnelCamera
{
public:
    enum{
        MSG_SENDFILEVIDEO
    };
    KeVideoSimulator(const std::string &fileName);
    virtual ~KeVideoSimulator();
    virtual bool Init(kaerp2p::PeerConnectionClientInterface *client);
    virtual void GetCameraVideoInfo(int level, kaerp2p::VideoInfo *info);
    void OnFileReadEnd();
    void OnFileVideoData(const char * data,int len);
    void OnFileAudioData(const char * data,int len);
protected:
    void OnRecvRecordQuery(std::string peer_id, std::string condition);

protected:
    kaerp2p::RecordReaderAvi * reader;
    std::string fileName;
};

#endif // KEVIDEOSIMULATOR_H
