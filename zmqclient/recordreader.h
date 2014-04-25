#ifndef RECORDREADER_H
#define RECORDREADER_H

#include "talk/base/sigslot.h"
#include "talk/base/messagehandler.h"


namespace talk_base {
class Thread;
class Buffer;
class FileStream;
}

class RecordReaderInterface:public talk_base::MessageHandler
{
public:
    enum{
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
    RecordReaderInterface();
private:
    talk_base::Thread * file_thread_;
    int frame_internal_;//internal millisecond

};

class FileRecordReader:public RecordReaderInterface
{
public:
    FileRecordReader(std::string filename);
protected:
    virtual talk_base::Buffer *ReadRecordFrame();
    virtual bool OpenRecord();
    virtual void CloseRecord();
private:
    std::string filename_;
    talk_base::FileStream * file_stream_;
    int file_pos;
    int last_frame;
};




#endif // RECORDREADER_H
