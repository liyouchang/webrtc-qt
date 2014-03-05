#ifndef STREAMPROCESS_H
#define STREAMPROCESS_H

#include "talk/app/kaerp2p/KaerSession.h"
#include <deque>
#include <queue>
#include "talk/base/bytebuffer.h"

class StreamProcess:public sigslot::has_slots<>,public talk_base::MessageHandler
{
public:
    enum{
        MSG_DATAWRITE
    };
    StreamProcess();

    bool ProcessStream(talk_base::StreamInterface* stream);
    void OnStreamEvent(talk_base::StreamInterface* stream, int events,
                       int error);
    void Cleanup(talk_base::StreamInterface* stream, bool delay = false);
    bool WriteData(const char * data,int len);

protected:
    //maybe change the buffer in case buffer is not sent all
    bool WriteBuffer(talk_base::ByteBuffer &buffer);

    char buffer_[1024 * 64];
    size_t buffer_len_;

    talk_base::StreamInterface *stream_;
    std::queue<talk_base::Buffer> writeQueue_;
    talk_base::Thread * workThread_;
    // MessageHandler interface
public:
    void OnMessage(talk_base::Message *msg);
};

#endif // STREAMPROCESS_H
