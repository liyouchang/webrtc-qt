#ifndef STREAMPROCESS_H
#define STREAMPROCESS_H

#include "talk/app/kaerp2p/KaerSession.h"
#include <deque>
#include <queue>
#include "talk/base/bytebuffer.h"
#include "talk/base/stream.h"
namespace kaerp2p {

class P2PStreamNotify
{
public:
    virtual void OnReadData(StreamProcess * stream,size_t readable_bytes);

};

class StreamProcess:public sigslot::has_slots<>,public talk_base::MessageHandler
{
public:
    enum{
        MSG_DATAWRITE
    };
    StreamProcess(talk_base::Thread *stream_thread,P2PStreamNotify * notify);

    bool ProcessStream(talk_base::StreamInterface* stream);

    bool WriteData(const char * data,int len);
    bool WriteBuffer(const talk_base::Buffer &buffer);
    void Cleanup();
    bool WriteStream(const char * data,int len);
    bool ReadStream(void *buffer, size_t bytes, size_t *bytes_read);
protected:
    virtual void OnReadBuffer(talk_base::Buffer &buffer);


    void Cleanup(talk_base::StreamInterface* stream, bool delay = false);
    void OnStreamEvent(talk_base::StreamInterface* stream, int events,
                       int error);
    void ReadStreamInternel();
    void WriteStreamInternel();
    talk_base::StreamInterface *stream_;
    std::queue<talk_base::Buffer> writeQueue_;
    talk_base::Thread * stream_thread_;
    // MessageHandler interface
    size_t totalread;
    talk_base::FifoBuffer read_buf_;
    talk_base::FifoBuffer write_buf_;
    P2PStreamNotify * notify_;
};

}
#endif // STREAMPROCESS_H
