#ifndef KEVIDEOSIMULATOR_H
#define KEVIDEOSIMULATOR_H

#include "talk/base/thread.h"
#include "talk/base/fileutils.h"
#include "talk/base/buffer.h"
#include "peerterminal.h"
class KeVideoSimulator:public talk_base::MessageHandler
{
public:
    KeVideoSimulator();
    ~KeVideoSimulator();
    bool ReadVideoData(std::string file_name);
    void SetTerminal(std::string peer_id,PeerTerminalInterface * t);
protected:
    PeerTerminalInterface * terminal_;
    talk_base::Thread * file_thread_;
    talk_base::Buffer video_data_;
};

#endif // KEVIDEOSIMULATOR_H
