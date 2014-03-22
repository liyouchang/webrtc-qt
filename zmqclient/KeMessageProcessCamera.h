#ifndef KEMESSAGEPROCESSCAMERA_H
#define KEMESSAGEPROCESSCAMERA_H

#include "peerterminal.h"
class KeMessageProcessCamera
{
public:
    KeMessageProcessCamera();
    sigslot::signal0<> SignalRecvAskVideoMsg;
    void SetTerminal(std::string peer_id,PeerTerminalInterface * t);
    void OnTunnelMessage(const std::string & peer_id,talk_base::Buffer & msg);
    void SendMediaMsg(const char *data, int len);

protected:
    void ExtractMessage(talk_base::Buffer & allBytes);
    void OnMessageRespond(talk_base::Buffer & msgData);


    std::string peer_id_;
    PeerTerminalInterface * terminal_;
    char headBuf[11];
    int bufPos;
    int toRead;
    talk_base::Buffer msgRecv;
    static const int msgMaxLen = 8192;
};

#endif // KEMESSAGEPROCESSCAMERA_H
