#ifndef KEMESSAGEPROCESSCAMERA_H
#define KEMESSAGEPROCESSCAMERA_H

#include "peerterminal.h"
#include "KeMessage.h"

class KeMessageProcessCamera:public sigslot::has_slots<>
{
public:
    KeMessageProcessCamera();
    sigslot::signal2<int,int> SignalRecvAskMediaMsg;
    void SetTerminal(std::string peer_id,PeerTerminalInterface * t);
    void OnTunnelMessage(const std::string & peer_id,talk_base::Buffer & msg);
    void SendMediaMsg(int type,const char *data, int len);

protected:
    void ExtractMessage(talk_base::Buffer & allBytes);
    void OnMessageRespond(talk_base::Buffer & msgData);


    void RecvAskMediaMsg(talk_base::Buffer &msgData);
    std::string peer_id_;
    PeerTerminalInterface * terminal_;
    char headBuf[11];
    int bufPos;
    int toRead;
    talk_base::Buffer msgRecv;
    static const int msgMaxLen = 8192;
};

#endif // KEMESSAGEPROCESSCAMERA_H
