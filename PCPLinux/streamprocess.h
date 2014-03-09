#ifndef STREAMPROCESS_H
#define STREAMPROCESS_H

#include "talk/app/kaerp2p/KaerSession.h"
#include <deque>
#include <queue>
#include "talk/base/bytebuffer.h"

namespace kaerp2p {

class StreamProcess:public sigslot::has_slots<>,public talk_base::MessageHandler
{
public:
    enum{
        MSG_DATAWRITE
    };
    StreamProcess(talk_base::Thread *stream_thread);

    bool ProcessStream(talk_base::StreamInterface* stream);

    bool WriteData(const char * data,int len);
    bool WriteBuffer(const talk_base::Buffer &buffer);
    void Cleanup();
protected:
    virtual void OnReadBuffer(talk_base::Buffer &buffer);


    void Cleanup(talk_base::StreamInterface* stream, bool delay = false);
    void OnStreamEvent(talk_base::StreamInterface* stream, int events,
                       int error);

    talk_base::StreamInterface *stream_;
    std::queue<talk_base::Buffer> writeQueue_;
    talk_base::Thread * stream_thread_;
    // MessageHandler interface
    size_t totalread;
public:
    virtual void OnMessage(talk_base::Message *msg);
};

}
#endif // STREAMPROCESS_H
