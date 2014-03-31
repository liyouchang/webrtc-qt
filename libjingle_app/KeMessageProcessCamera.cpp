#include "KeMessageProcessCamera.h"

KeMessageProcessCamera::KeMessageProcessCamera(std::string peer_id):
    KeMsgProcess(peer_id),start_video_(false)
{
}


void KeMessageProcessCamera::OnMessageRespond(talk_base::Buffer &msgData)
{
    char msgType = msgData.data()[1];
    switch(msgType){
    case KEMSG_TYPE_VIDEOSERVER:
        RecvAskMediaMsg(msgData);
        break;
    }

}

void KeMessageProcessCamera::RecvAskMediaMsg(talk_base::Buffer &msgData)
{
    //send stream
    LOG(INFO)<< __FUNCTION__<<"receive message video server msg";
    KEVideoServerReq * msg = (KEVideoServerReq *)msgData.data();
    int video = msg->video;

    if(video == 0){
        start_video_ = true;
    }

    int audio = msg->listen;
    SignalRecvAskMediaMsg(this,video,audio);

}

void KeMessageProcessCamera::OnVideoData(const char *data, int len)
{
    talk_base::Buffer sendBuf;
    int msgLen = sizeof(KERTStreamHead)  + len;
    sendBuf.SetLength(msgLen);
    KERTStreamHead * head = (KERTStreamHead *)sendBuf.data();
    head->protocal = PROTOCOL_HEAD;
    head->msgType = KEMSG_TYPE_VIDEOSTREAM;
    head->msgLength = msgLen;
    head->channelNo = 1;
    head->videoID = 0;
    memcpy(sendBuf.data() + sizeof(KERTStreamHead),data,len);

    SignalNeedSendData(this->peer_id_,sendBuf.data(),sendBuf.length());

}

void KeMessageProcessCamera::OnAudioData(const char *data, int len)
{
    talk_base::Buffer sendBuf;
    int msgLen = sizeof(KERTStreamHead)  + len;
    sendBuf.SetLength(msgLen);
    KERTStreamHead * head = (KERTStreamHead *)sendBuf.data();
    head->protocal = PROTOCOL_HEAD;
    head->msgType = KEMSG_TYPE_AUDIOSTREAM;
    head->msgLength = msgLen;
    head->channelNo = 1;
    head->videoID = 0;
    memcpy(sendBuf.data() + sizeof(KERTStreamHead),data,len);

    SignalNeedSendData(this->peer_id_,sendBuf.data(),sendBuf.length());

}

