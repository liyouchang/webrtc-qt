#ifndef KEMSGPROCESS_H
#define KEMSGPROCESS_H

#include "talk/base/buffer.h"
#include <string>
#include "talk/base/sigslot.h"
class KeMsgProcess:public sigslot::has_slots<>
{
public:
    KeMsgProcess(std::string peer_id);
    sigslot::signal3<const std::string &,const char * ,int> SignalNeedSendData;
    void OnTunnelMessage(const std::string & peer_id,talk_base::Buffer & msg);
    std::string GetPeerID(){ return peer_id_;}

protected:
    virtual void ExtractMessage(talk_base::Buffer & allBytes);
    virtual void OnMessageRespond(talk_base::Buffer & msgData);


    std::string peer_id_;
    //PeerTerminalInterface * terminal_;
    char headBuf[11];
    int bufPos;
    int toRead;
    talk_base::Buffer msgRecv;
    const int msgMaxLen;

};


class KeMessageProcessCamera: public KeMsgProcess
{
public:
    KeMessageProcessCamera(std::string peer_id);
    sigslot::signal3<KeMessageProcessCamera *,int,int> SignalRecvAskMediaMsg;
    void OnVideoData(const char *data, int len);
    void OnAudioData(const char * data,int len);
protected:
    virtual void OnMessageRespond(talk_base::Buffer & msgData);
    void RecvAskMediaMsg(talk_base::Buffer &msgData);

    bool start_video_;
};


#endif // KEMSGPROCESS_H
