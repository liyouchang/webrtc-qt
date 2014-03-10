#ifndef P2PTHREAD_H
#define P2PTHREAD_H

//#include <QThread>
#include "talk/base/thread.h"
#include "ServerConductor.h"

#ifdef WIN32
#include "talk/base/win32socketinit.h"
#include "talk/base/win32socketserver.h"
#endif

namespace  kaerp2p {


enum {
    MSG_LOGIN_COMPLETE = 1,
    MSG_LOGIN_FAILED,
    MSG_DONE,
    MSG_CONNECT_TO_PEER,
    MSG_SEND_STRING,
    MSG_WRITE_BUFFER,
    MSG_SEND_TO_PEER
};


class P2PThread : public talk_base::MessageHandler,
        public talk_base::Thread
{
public:
    explicit P2PThread();
    talk_base::scoped_refptr<kaerp2p::ServerConductor> conductor_;
    PeerConnectionClient * client_;

public:
    virtual void Run();

    // Thread interface

    // MessageHandler interface
public:
    void OnMessage(talk_base::Message *msg);
};
}
#endif // P2PTHREAD_H
