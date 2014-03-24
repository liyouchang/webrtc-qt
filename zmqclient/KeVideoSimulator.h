#ifndef KEVIDEOSIMULATOR_H
#define KEVIDEOSIMULATOR_H

#include "talk/base/thread.h"
#include "talk/base/fileutils.h"
#include "talk/base/buffer.h"
#include "peerterminal.h"
#include "KeMessageProcessCamera.h"
class KeVideoSimulator:public talk_base::MessageHandler,public sigslot::has_slots<>
{
public:
    enum{
        MSG_SENDFILEVIDEO
    };
    KeVideoSimulator();
    ~KeVideoSimulator();
    bool ReadVideoData(std::string file_name);
    void OnTunnelOpened(PeerTerminalInterface * t,const std::string & peer_id);
    void OnTunnelClosed(PeerTerminalInterface * t,const std::string & peer_id);

    void SendMediaMsg(const char *data, int len);
    void OnMediaRequest(int video,int audio);
protected:

    int MediaRequest_m(int video,int audio);
    talk_base::scoped_ptr<KeMessageProcessCamera> process_;
    talk_base::Thread * media_thread_;
    talk_base::Buffer video_data_;

    bool startSend;
    // MessageHandler interface
public:
    void OnMessage(talk_base::Message *msg);
};

#endif // KEVIDEOSIMULATOR_H
