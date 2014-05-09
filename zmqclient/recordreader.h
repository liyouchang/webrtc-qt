#ifndef RECORDREADER_H
#define RECORDREADER_H

#include <string>
#include "talk/base/sigslot.h"
#include "talk/base/messagehandler.h"


namespace talk_base {
class Thread;
class Buffer;
class FileStream;
}

struct KEFrameHead;

class RecordReaderInterface:public talk_base::MessageHandler
{
public:
    enum {
        MSG_RECORD_SEND
    };
    // MessageHandler interface
    virtual void OnMessage(talk_base::Message *msg);
    virtual bool StartPlayRecord(int internal,talk_base::Thread *thread);
    sigslot::signal2<const char * ,int> SignalRecordData;
protected:
    virtual talk_base::Buffer *ReadRecordFrame() = 0;
    virtual bool OpenRecord() = 0;
    virtual void CloseRecord() = 0;
    RecordReaderInterface(int frame_internal);
    int frameInternal_;//internal millisecond
    bool useInterval;

private:
    talk_base::Thread * file_thread_;


};

class FileRecordReader:public RecordReaderInterface
{
public:
    FileRecordReader(std::string filename,int frame_internal);
    virtual ~FileRecordReader();
protected:
    virtual talk_base::Buffer *ReadRecordFrame();
    virtual bool OpenRecord();
    virtual void CloseRecord();
private:
    std::string filename_;
    talk_base::FileStream * file_stream_;
    int file_pos;
    int last_frame;
    KEFrameHead * sendFrameHead_;
    int send_speed_;

};




#endif // RECORDREADER_H
