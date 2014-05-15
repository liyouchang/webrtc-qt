#ifndef RECORDERAVI_H
#define RECORDERAVI_H

#include <string>
#include "talk/base/sigslot.h"
namespace talk_base{
class Buffer;
class FileStream;
}

class RecorderAvi:public sigslot::has_slots<>
{
public:
    RecorderAvi(const std::string &peerId, int frameRate, int frameType);
    virtual ~RecorderAvi();
    bool StartRecord(const std::string & fileName);
    bool StopRecord();
    void OnVideoData(const std::string & peerId,const char *data,int len);
    void OnAudioData(const std::string & peerId,const char *data,int len);

protected:
    bool AviFileWriteVideo(const char *mediaData, int mediaLen);
    bool AviFileWriteAudio(const char *mediaData, int mediaLen);

private:
    std::string peerId_;
    int frameRate_;//for example 25 frame/s
    int frameType_;
    int moviListLen;
    int avifileLen;
    bool recvIFrame;
    talk_base::Buffer * aviIndex_;
    talk_base::FileStream * aviFile_;


};

#endif // RECORDERAVI_H
