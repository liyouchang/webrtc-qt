#ifndef RECORDERAVI_H
#define RECORDERAVI_H

#include <string>

namespace talk_base{
class Buffer;
class FileStream;
}

class RecorderAvi
{
public:
    RecorderAvi();
    virtual ~RecorderAvi();
    bool StartRecord(const std::string & fileName);
    void OnVideoData(const std::string & peerId,const char *data,int len);
    void OnAudioData(const std::string & peerId,const char *data,int len);
private:

private:
    std::string peerId_;
    int frameRate_;//25 frame/s
    int frameType_;
    talk_base::Buffer * aviIndex_;
    talk_base::FileStream * aviFile_;

};

#endif // RECORDERAVI_H
