#ifndef RECORDERAVI_H
#define RECORDERAVI_H
#include <string>

class talk_base::Buffer;
class talk_base::FileStream;


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
