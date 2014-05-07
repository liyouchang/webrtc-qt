#include "KeMsgProcess.h"
#include "talk/base/stream.h"
#include "talk/base/thread.h"
#include "talk/base/stringutils.h"



#include "KeMessage.h"
#include "KeMsgProcessContainer.h"

const int kHeartStop = 5; //5 time without receive heart
const int kHeartDelay = 1000;  // 1000 milliseconds


KeMsgProcess::KeMsgProcess(std::string peer_id, KeMsgProcessContainer *container):
    peer_id_(peer_id),kMsgMaxLen(512*1024),heart_thread_(0),heart_count_(0),
    container_(container)
{
    buf_position_ = 0;
    to_read_ = 0;
}

void KeMsgProcess::OnTunnelMessage(const std::string &peer_id,
                                   talk_base::Buffer &msg)
{
    ASSERT(this->peer_id_ == peer_id);
    ASSERT(msg.length() > 0);
    this->ExtractMessage(msg);
}

void KeMsgProcess::StartHeartBeat()
{
    LOG(INFO)<<"KeMsgProcess::StartHeartBeat";
    heart_thread_ =  talk_base::Thread::Current();
    this->heart_count_ = 0;
    SendHeart();
}

void KeMsgProcess::ExtractMessage(talk_base::Buffer &allBytes)
{
    talk_base::MemoryStream buffer(allBytes.data(),allBytes.length());
    const int headLen = 10;
    talk_base::StreamResult result;
    size_t read_bytes;
    int error;
    while(true)
    {
        if (msg_received_.length() == 0)//上一个消息已经读取完成
        {
            result = buffer.Read(head_buffer_+buf_position_,
                                 headLen-buf_position_,&read_bytes,&error);
            if(result == talk_base::SR_EOS){
                break;
            }
            //消息头在最后几个字节，记录读取的字节，下次继续读取。
            if(read_bytes < headLen-buf_position_)
            {
                //LOG(INFO)<<"Continue Read head in new package\r\n ";
                buf_position_ = read_bytes;
                break;
            }
            unsigned char  protocal = head_buffer_[0];
            //int msgLen= *((int*)&headBuf[2]); //arm will failed with this
            int msgLen;
            memcpy(&msgLen,&head_buffer_[2],4);
            if (protocal != PROTOCOL_HEAD ||  msgLen > kMsgMaxLen)
            {
                LOG(WARNING)<<"The message Protocal Head "<< read_bytes
                           <<" error, msg len "<<msgLen
                          <<" ,Clear the recv buffer!\r\n";
                msg_received_.SetLength(0);
                break;
            }
            msg_received_.SetLength(msgLen);
            buf_position_ = 0;
            memcpy(msg_received_.data(),head_buffer_,headLen);
            buf_position_ += headLen;
            to_read_ =  msgLen-headLen;
            if (to_read_ != 0)//防止 headLen 越界
            {
                result = buffer.Read(msg_received_.data()+buf_position_,
                                     to_read_,&read_bytes,&error);
                if(result == talk_base::SR_EOS){
                    break;
                }
                buf_position_ += read_bytes;
                to_read_ -= read_bytes;
            }
        }
        else//上一个消息未完成读取
        {
            result = buffer.Read(msg_received_.data()+buf_position_,
                                 to_read_,&read_bytes,&error);
            if(result == talk_base::SR_EOS){
                break;
            }
            if (read_bytes < to_read_){
                //LOG(INFO)<<"to read more and more!";
            }
            buf_position_ += read_bytes;
            to_read_ -= read_bytes;
        }
        if(to_read_ == 0 && buf_position_ == msg_received_.length())//全部读取
        {
            this->OnMessageRespond(msg_received_);
            msg_received_.SetLength(0);
            buf_position_ = 0;
        }
    }

}

void KeMsgProcess::OnMessageRespond(talk_base::Buffer &msgData)
{
    char msgType = msgData.data()[1];
    switch(msgType){
    case DevMsg_HeartBeat:
        if(heart_thread_){
            heart_thread_->Post(this,MSG_HEART_RECEIVED);
        }
        break;
    default:
        KeMsgProcess::OnMessageRespond(msgData);
        break;
    }


}

void KeMsgProcess::SendHeart()
{
    talk_base::Buffer sendBuf;
    int msgLen = sizeof(KEDevMsgHead);
    sendBuf.SetLength(msgLen);
    KEDevMsgHead * pReqMsg;
    pReqMsg = (KEDevMsgHead *)sendBuf.data();
    pReqMsg->protocal = PROTOCOL_HEAD;
    pReqMsg->msgType = DevMsg_HeartBeat;
    pReqMsg->msgLength = msgLen;
    pReqMsg->videoID = 0;

    SignalNeedSendData(this->peer_id_,sendBuf.data(),sendBuf.length());

    if(heart_thread_){
        heart_thread_->PostDelayed(kHeartDelay,this,MSG_HEART_SENDED);
    }
}


void KeMsgProcess::OnMessage(talk_base::Message *msg)
{
    switch(msg->message_id){
    case MSG_HEART_SENDED:{
        LOG(LS_VERBOSE)<<"heart beat "<<heart_count_;
        if(heart_count_++ > kHeartStop){
            SignalHeartStop(peer_id_);
            break;
        }
        SendHeart();
        break;
    }
    case MSG_HEART_RECEIVED:{
        heart_count_ = 0;
        break;
    }

    }
}


KeMessageProcessCamera::KeMessageProcessCamera(std::string peer_id,
                                               KeTunnelCamera *container):
    KeMsgProcess(peer_id,container),video_started_(false),audio_started_(false),
    talk_started_(false)
{

}


void KeMessageProcessCamera::OnMessageRespond(talk_base::Buffer &msgData)
{
    char msgType = msgData.data()[1];
    switch(msgType){
    case KEMSG_TYPE_VIDEOSERVER:
        RecvAskMediaMsg(msgData);
        break;
    case KEMSG_REQUEST_PLAY_FILE:
        RecvPlayFile(msgData);
        break;
    case KEMSG_TYPE_AUDIOSTREAM://receive talk data
        RecvTalkData(msgData);
        break;
    default:
        KeMsgProcess::OnMessageRespond(msgData);
        break;
    }

}

void KeMessageProcessCamera::RecvAskMediaMsg(talk_base::Buffer &msgData)
{
    //send stream
    KEVideoServerReq * msg = (KEVideoServerReq *)msgData.data();

    LOG(INFO)<< "KeMessageProcessCamera::RecvAskMediaMsg---"
             <<"receive ask media msg : video-"<<msg->video<<
               " listen-"<<msg->listen<<" talk-"<<msg->talk;

    KeTunnelCamera * camera = static_cast<KeTunnelCamera *>(container_);

    int video = msg->video;
    if(video == 0){//stop
        camera->SignalVideoData1.disconnect(this);
        camera->SignalVideoData2.disconnect(this);
        this->video_started_ = false;
    }
    else if(!video_started_){
        video_started_ = true;
        if(video == 1){
            camera->SignalVideoData1.connect(
                        this , &KeMessageProcessCamera::OnVideoData);
        }
        else if(video == 2){
            camera->SignalVideoData2.connect(
                        this , &KeMessageProcessCamera::OnVideoData);
        }
    }

    int audio = msg->listen;
    if(audio == 0){
        camera->SignalAudioData.disconnect(this);
        this->audio_started_ = false;
    }
    else if(!audio_started_){
        audio_started_ = true;
        camera->SignalAudioData.connect(
                    this,&KeMessageProcessCamera::OnAudioData);
    }

    int talk = msg->talk;
    if(talk == 0){
        this->SignalRecvTalkData.disconnect(camera);
        this->talk_started_ = false;

    }else if(!talk_started_){
        this->SignalRecvTalkData.connect(camera,&KeTunnelCamera::OnRecvTalkData);
        this->talk_started_ = true;
    }

}

void KeMessageProcessCamera::RecvPlayFile(talk_base::Buffer &msgData)
{
    KEPlayRecordFileReq * pMsg =
            reinterpret_cast<KEPlayRecordFileReq *>(msgData.data());
    LOG(INFO)<< "KeMessageProcessCamera::RecvPlayFile--"<<pMsg->fileData;

    this->SignalToPlayFile(this->peer_id(),pMsg->fileData);


}

void KeMessageProcessCamera::RecvTalkData(talk_base::Buffer &msgData)
{
    KERTStreamHead * pMsg = (KERTStreamHead *)msgData.data();
    const int sendStartPos = 11;
    int mediaDataLen = msgData.length() - sendStartPos;
    if(pMsg->msgType == KEMSG_TYPE_AUDIOSTREAM){
        SignalRecvTalkData(this->peer_id(),msgData.data() +
                            sendStartPos,mediaDataLen);
    }else{
        LOG(WARNING)<<"KeMessageProcessCamera::RecvTalkData---"<<
                    "message type error";
    }
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

    SignalNeedSendData(this->peer_id(),sendBuf.data(),sendBuf.length());

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

    SignalNeedSendData(this->peer_id(),sendBuf.data(),sendBuf.length());

}



KeMessageProcessClient::KeMessageProcessClient(std::string peer_id,
                                               KeTunnelClient * container):
    KeMsgProcess(peer_id,container)
{

}

void KeMessageProcessClient::AskVideo(int video, int listen, int talk)
{
    talk_base::Buffer sendBuf;
    int msgLen = sizeof(KEVideoServerReq);
    sendBuf.SetLength(msgLen);
    KEVideoServerReq * pReqMsg;
    pReqMsg = (KEVideoServerReq *)sendBuf.data();
    pReqMsg->protocal = PROTOCOL_HEAD;
    pReqMsg->msgType = KEMSG_TYPE_VIDEOSERVER;
    pReqMsg->msgLength = msgLen;
    pReqMsg->clientID = 0;
    pReqMsg->channelNo = 1;
    pReqMsg->videoID = 1;
    pReqMsg->video = video;
    pReqMsg->listen = listen;
    pReqMsg->talk = talk;
    pReqMsg->protocalType = 0;
    pReqMsg->transSvrIp = 0;

    SignalNeedSendData(this->peer_id(),sendBuf.data(),sendBuf.length());

}

void KeMessageProcessClient::ReqestPlayFile(const char *file_name)
{
    talk_base::Buffer sendBuf;
    int msgLen = sizeof(KEPlayRecordFileReq);
    sendBuf.SetLength(msgLen);
    KEPlayRecordFileReq * pReqMsg;
    pReqMsg = (KEPlayRecordFileReq *)sendBuf.data();
    pReqMsg->protocal = PROTOCOL_HEAD;
    pReqMsg->msgType = KEMSG_REQUEST_PLAY_FILE;
    pReqMsg->msgLength = msgLen;
    pReqMsg->clientID = 0;
    pReqMsg->channelNo = 1;
    pReqMsg->videoID = 0;
    pReqMsg->protocalType = 0;
    talk_base::strcpyn(pReqMsg->fileData,80,file_name);

    SignalNeedSendData(this->peer_id(),sendBuf.data(),sendBuf.length());

}
//send talk data to camera
void KeMessageProcessClient::OnTalkData(const char *data, int len)
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

    SignalNeedSendData(this->peer_id(),sendBuf.data(),sendBuf.length());

}

void KeMessageProcessClient::OnMessageRespond(talk_base::Buffer &msgData)
{
    char msgType = msgData.data()[1];
    switch(msgType)
    {
    case KEMSG_TYPE_VIDEOSERVER:
        break;
    case KEMSG_TYPE_MEDIATRANS:
        break;
    case KEMSG_TYPE_AUDIOSTREAM:
    case KEMSG_TYPE_VIDEOSTREAM:{
        RecvMediaData(msgData);
        break;
    }
    case KEMSG_RecordPlayData:{
        OnRecvRecordMsg(msgData);
        break;
    }
    default:
        KeMsgProcess::OnMessageRespond(msgData);
        break;
    }

}

void KeMessageProcessClient::OnRecvRecordMsg(talk_base::Buffer &msgData)
{
    KEPlayRecordDataHead * phead = (KEPlayRecordDataHead *)msgData.data();
    KeTunnelClient *client  = static_cast< KeTunnelClient *>(container_);

    if(phead->resp == 13){
        const int data_pos = sizeof(KEPlayRecordDataHead);
        int mediaDataLen = msgData.length() - data_pos;
        client->OnRecordFileData(
                    this->peer_id(),msgData.data()+data_pos,mediaDataLen);
    }else if(phead->resp == 6){
        client->OnRecordStatus(this->peer_id(),6);
    }else if(phead->resp == 5){
        client->OnRecordStatus(this->peer_id(),5);
    }else if(phead->resp == 4){
        client->OnRecordStatus(this->peer_id(),4);
    }
}

void KeMessageProcessClient::RecvMediaData(talk_base::Buffer &msgData)
{
    KERTStreamHead * pMsg = (KERTStreamHead *)msgData.data();
    const int sendStartPos = 11;
    int mediaDataLen = msgData.length() - sendStartPos;
    if(pMsg->msgType == KEMSG_TYPE_VIDEOSTREAM){
        SignalRecvVideoData(this->peer_id(),msgData.data() +
                            sendStartPos,mediaDataLen);
    }else if(pMsg->msgType == KEMSG_TYPE_AUDIOSTREAM){
        SignalRecvAudioData(this->peer_id(),msgData.data() +
                            sendStartPos,mediaDataLen);
    }
}
