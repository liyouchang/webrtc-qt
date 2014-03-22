#ifndef HISIMEDIADEVICE_H
#define HISIMEDIADEVICE_H


#include "talk/base/buffer.h"
#include "peerterminal.h"
#include "talk/base/thread.h"
#include "KeMessageProcessCamera.h"
class HisiMediaDevice
{
public:
    HisiMediaDevice();

    void OnTunnelOpend(PeerTerminalInterface * t,const std::string & peer_id);
private:
    talk_base::Thread * media_thread_;

    KeMessageProcessCamera * process_;
};

#endif // HISIMEDIADEVICE_H
