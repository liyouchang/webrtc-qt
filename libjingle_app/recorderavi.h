/*
 *
 * Copyright 2014 Kaer Electric Co.,Ltd
 * Listens see <http://www.gnu.org/licenses/>.
 * AUTHORS lht
 *
 * RecorderAvi 实现了视频数据存储成avi文件的功能
 *
 */

#ifndef RECORDERAVI_H
#define RECORDERAVI_H

#include "talk/base/thread.h"

#include "recordinterface.h"

namespace talk_base{
class Buffer;
class FileStream;
}

namespace kaerp2p {



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
    int frameCount;
};

class RecordReaderAvi :
        public RecordReaderInterface,
        public talk_base::MessageHandler
{
public:
    RecordReaderAvi(int audioInterval = 20,talk_base::Thread * thread = NULL);
    virtual ~RecordReaderAvi();
    void OnMessage(talk_base::Message *msg);

    bool StartRead(const std::string & filename);
    bool StopRead();
private:
    talk_base::FileStream * aviFile_;
    talk_base::Thread * readThread;
    int audioFrameInterval;//million second, audio frame interval time,
                            //20 is normal speed
    bool ownThread;

public:
};

}
#endif // RECORDERAVI_H
