#ifndef KEVIDEOSIMULATOR_H
#define KEVIDEOSIMULATOR_H

#include "libjingle_app/ketunnelcamera.h"
#include "talk/base/messagehandler.h"
#include "libjingle_app/recorderavi.h"


#include "zmq.h"
#include "zmq.hpp"
#include "zhelpers.hpp"
#include "zmsg.hpp"

namespace talk_base {
    class Thread;
    class Buffer;
}

class KeVideoSimulator:public kaerp2p::KeTunnelCamera,public talk_base::MessageHandler
{
public:
    enum {
        MSG_SENDFILEVIDEO,
        MSG_ZMQ_RECV

    };
    KeVideoSimulator(const std::string &fileName);
    virtual ~KeVideoSimulator();
    virtual bool Init(kaerp2p::PeerTerminalInterface *t);
    virtual void GetCameraVideoInfo(int level, kaerp2p::VideoInfo *info);
    void OnFileReadEnd(kaerp2p::RecordReaderInterface *);
    void OnFileVideoData(const char * data,int len);
    void OnFileAudioData(const char * data,int len);

    void OnMessage(talk_base::Message *msg);

protected:
    void OnCommandJsonMsg(const Json::Value &jmessage,Json::Value *jresult);

protected:
    kaerp2p::RecordReaderInterface * reader;
    std::string fileName;

    zmq::context_t *zmqContext;
    zmq::socket_t *publisher;

    zmq::socket_t *repSocket;

    talk_base::Thread *zmqThread;

};

#endif // KEVIDEOSIMULATOR_H
