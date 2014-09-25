#ifndef RECORDERMP4_H
#define RECORDERMP4_H


#include "recordinterface.h"
#include "mp4encoder.h"

namespace kaerp2p {

class RecorderMp4 : public RecordSaverInterface
{
public:
    RecorderMp4(const std::string &peerId, int frameRate, int width,int height);
    ~RecorderMp4();
    // RecordSaverInterface interface
public:
    bool StartSave(const std::string &fileName);
    bool StopSave();
    void OnVideoData(const std::string &peerId, const char *data, int len);
    void OnAudioData(const std::string &peerId, const char *data, int len);
private:
    MP4Encoder * mp4;
    std::string peerId;
    int frameRate;//for example 25 frame/s
    int frameHeight;
    int frameWidth;
    MP4FileHandle hMp4File;
};

}
#endif // RECORDERMP4_H
