#ifndef STREAMPROCESS_H
#define STREAMPROCESS_H

#include "talk/app/kaerp2p/KaerSession.h"
#include <deque>
#include <queue>
#include "talk/base/bytebuffer.h"
#include "talk/base/stream.h"

namespace kaerp2p {


class StreamProcess:public sigslot::has_slots<>
{
public:
    enum{
        MSG_DATAWRITE
    };
    StreamProcess(talk_base::Thread *stream_thread);

    bool ProcessStream(talk_base::StreamInterface* stream);

    void Cleanup();
    bool WriteStream(const char * data,int len);
    bool ReadStream(void *buffer, size_t bytes, size_t *bytes_read);

    sigslot::signal2<StreamProcess*, size_t> SignalReadData;

protected:


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
};

}
#endif // STREAMPROCESS_H
