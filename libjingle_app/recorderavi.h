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

#include <vector>

namespace talk_base{
class Buffer;
class FileStream;
}

namespace kaerp2p {

typedef struct {
    uint32_t ckid; //记录数据块中子块的标记
    uint32_t dwFlags; //表示chid所指子块的属性
    uint32_t dwChunkOffset; //子块的相对位置
    uint32_t dwChunkLength; //子块长度
}AVIINDEXENTRY;


class RecorderAvi : public RecordSaverInterface
{
public:
    RecorderAvi(const std::string &peerId, int frameRate, int frameType);
    virtual ~RecorderAvi();
    virtual bool StartSave(const std::string & fileName);
    virtual bool StopSave();
    virtual void OnVideoData(const std::string & peerId,const char *data,int len);
    virtual void OnAudioData(const std::string & peerId,const char *data,int len);
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
    enum {
        MSG_READ,
        MSG_SETSPEED,
        MSG_SETPOSITION
    };
    void OnMessage(talk_base::Message *msg);
    bool StartRead(const std::string & filename);
    bool StopRead();
    bool IsReading();

    bool SetSpeed(int speed);//0x10 normal ,0x20 2x speed,0x40 4x speed,
                            //0x08 1/2 speed,0x04 1/4 speed,0 pause,100 continue
    int GetSpeed();
    bool SetPosition(int percent);// 0-100 percent
    int GetPosition();
    sigslot::signal1<int> SignalReportProgress;
private:
    void MoveTo(int percent);
    void ReadRecord();
    int GetPlayedPercent();
    talk_base::FileStream * aviFile_;
    talk_base::Thread * readThread;
    int audioFrameInterval;//million second, audio frame interval time,
                            //20 is normal speed
    int speed; //0x10 normal,0x20 2x speed,0x40 4x speed,0x08 1/2 speed,0x04 1/4 speed
    int totalFrame;
    int currentFrame;
    int oldPercent;
    int indexPos;
    bool ownThread;
    std::vector<AVIINDEXENTRY> recordIndex;
};

class FakeRecordReaderAvi: public RecordReaderInterface,public talk_base::MessageHandler
{
public:
    FakeRecordReaderAvi(int interval);
    ~FakeRecordReaderAvi();
    void OnMessage(talk_base::Message *msg);
    bool StartRead(const std::string &filename);
    bool StopRead();
protected:
    talk_base::Thread * readThread;
    int interval;
};


}
#endif // RECORDERAVI_H
