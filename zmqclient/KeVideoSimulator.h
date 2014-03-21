#ifndef KEVIDEOSIMULATOR_H
#define KEVIDEOSIMULATOR_H

#include "talk/base/thread.h"
#include "talk/base/fileutils.h"
#include "talk/base/buffer.h"
#include "peerterminal.h"
class KeVideoSimulator:public talk_base::MessageHandler,public sigslot::has_slots<>
{
public:
    enum{
        MSG_SENDFILEVIDEO
    };
    KeVideoSimulator();
    ~KeVideoSimulator();
    bool ReadVideoData(std::string file_name);
    void SetTerminal(std::string peer_id,PeerTerminalInterface * t);

    void OnTunnelMessage(const std::string & peer_id,talk_base::Buffer & msg);
    void OnTunnelOpend(PeerTerminalInterface * t,const std::string & peer_id);
    sigslot::signal0<> SignalRecvAskVideoMsg;

    void OnSendVideo();
protected:
    void ExtractMessage(talk_base::Buffer & allBytes);
    void OnMessageRespond(talk_base::Buffer & msgData);

    std::string peer_id_;
    PeerTerminalInterface * terminal_;
    talk_base::Thread * file_thread_;
    talk_base::Buffer video_data_;
    char headBuf[11];
    int bufPos;
    int toRead;
    talk_base::Buffer msgRecv;
    static const int msgMaxLen = 8192;
    bool startSend;
    // MessageHandler interface
public:
    void OnMessage(talk_base::Message *msg);
};

#endif // KEVIDEOSIMULATOR_H
