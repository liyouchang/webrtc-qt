#ifndef P2PTHREAD_H
#define P2PTHREAD_H

//#include <QThread>
#include "talk/base/thread.h"
#include "ServerConductor.h"

namespace  kaerp2p {


enum {
    MSG_LOGIN_COMPLETE = 1,
    MSG_LOGIN_FAILED,
    MSG_DONE,
    MSG_CONNECT_TO_PEER,
    MSG_SEND_STRING,
    MSG_WRITE_BUFFER
};


class P2PThread : public talk_base::Thread,public talk_base::MessageHandler
{
public:
    explicit P2PThread();
    talk_base::scoped_refptr<kaerp2p::ServerConductor> conductor_;
public:
    virtual void Run();

    // Thread interface

    // MessageHandler interface
public:
    void OnMessage(talk_base::Message *msg);
};
}
#endif // P2PTHREAD_H
