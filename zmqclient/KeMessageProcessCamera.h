#ifndef KEMESSAGEPROCESSCAMERA_H
#define KEMESSAGEPROCESSCAMERA_H

#include "peerterminal.h"
#include "KeMessage.h"
#include <string>
class KeMessageProcessCamera:public sigslot::has_slots<>
{
public:
    KeMessageProcessCamera(std::string peer_id);
    sigslot::signal3<KeMessageProcessCamera *,int,int> SignalRecvAskMediaMsg;
    sigslot::signal3<const std::string &,const char * ,int> SignalNeedSendData;
    //void SetTerminal(std::string peer_id,PeerTerminalInterface * t);
    void OnTunnelMessage(const std::string & peer_id,talk_base::Buffer & msg);
    void OnVideoData(const char *data, int len);
    void OnAudioData(const char * data,int len);
    //void SendMediaMsg(int type,const char *data, int len);
    std::string GetPeerID(){ return peer_id_;}
protected:
    void ExtractMessage(talk_base::Buffer & allBytes);
    void OnMessageRespond(talk_base::Buffer & msgData);
    void RecvAskMediaMsg(talk_base::Buffer &msgData);

    bool start_video_;

    std::string peer_id_;
    //PeerTerminalInterface * terminal_;
    char headBuf[11];
    int bufPos;
    int toRead;
    talk_base::Buffer msgRecv;
    static const int msgMaxLen = 8192;
};

#endif // KEMESSAGEPROCESSCAMERA_H
