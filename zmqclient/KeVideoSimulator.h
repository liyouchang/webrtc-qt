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

    void SetTerminal(PeerTerminalInterface *t);
    bool ReadVideoData(std::string file_name);
    void OnTunnelOpened(PeerTerminalInterface * t,const std::string & peer_id);
    void OnTunnelClosed(PeerTerminalInterface * t,const std::string & peer_id);
    void OnTunnelMessage(const std::string &peer_id, talk_base::Buffer &msg);
    void OnRouterMessage(const std::string &peer_id,const std::string& msg);
    void SendMediaMsg(const char *data, int len);
    KeMessageProcessCamera * GetProcess(const std::string & peer_id);

    sigslot::signal2<const char *, int > SignalVideoData;
    sigslot::signal2<const char *, int > SignalAudioData;
protected:

    void OnProcessMediaRequest(KeMessageProcessCamera * process,int video,int audio);

    void OnProcessNeedSend(const std::string & peer_id,const char * data,int len);


    //talk_base::scoped_ptr<KeMessageProcessCamera> process_;
    talk_base::Thread * media_thread_;
    talk_base::Buffer video_data_;
    std::vector<KeMessageProcessCamera *> processes_;
    //bool startSend;

    PeerTerminalInterface * terminal_;
    // MessageHandler interface
public:
    void OnMessage(talk_base::Message *msg);
};

#endif // KEVIDEOSIMULATOR_H
