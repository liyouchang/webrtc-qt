#include "KeMsgProcess.h"
#include "talk/base/stream.h"
#include "KeMessage.h"
#include "talk/base/thread.h"


const int kHeartStop = 3; //3 time without receive heart
const int kHeartDelay = 1000;  // 1000 milliseconds


KeMsgProcess::KeMsgProcess(std::string peer_id):
    peer_id_(peer_id),kMsgMaxLen(512*1024),heart_thread_(0),heart_count_(0)
{
    buf_position_ = 0;
    to_read_ = 0;
}

void KeMsgProcess::OnTunnelMessage(const std::string &peer_id, talk_base::Buffer &msg)
{
    ASSERT(this->peer_id_ == peer_id);
    ASSERT(msg.length() > 0);

    this->ExtractMessage(msg);
}

void KeMsgProcess::StartHeartBeat()
{
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
            result = buffer.Read(head_buffer_+buf_position_,headLen-buf_position_,&read_bytes,&error);
            if(result == talk_base::SR_EOS){
                break;
            }
            if(read_bytes < headLen-buf_position_)//消息头在最后几个字节，记录读取的字节，下次继续读取。
            {
                LOG(INFO)<<"Continue Read head in new package\r\n ";
                buf_position_ = read_bytes;
                break;
            }
            unsigned char  protocal = head_buffer_[0];
            //int msgLen= *((int*)&headBuf[2]); //arm will failed with this
            int msgLen;
            memcpy(&msgLen,&head_buffer_[2],4);
            if (protocal != PROTOCOL_HEAD ||  msgLen > kMsgMaxLen)
            {
                LOG(WARNING)<<"The message Protocal Head "<< read_bytes <<" error, msg len "
                           <<msgLen<<" ,Clear the recv buffer!\r\n";
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
                result = buffer.Read(msg_received_.data()+buf_position_,to_read_,&read_bytes,&error);
                if(result == talk_base::SR_EOS){
                    break;
                }
                buf_position_ += read_bytes;
                to_read_ -= read_bytes;
            }
        }
        else//上一个消息未完成读取
        {
            result = buffer.Read(msg_received_.data()+buf_position_,to_read_,&read_bytes,&error);
            if(result == talk_base::SR_EOS){
                break;
            }
            if (read_bytes < to_read_){
                LOG(INFO)<<"to read more and more!";
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


KeMessageProcessCamera::KeMessageProcessCamera(std::string peer_id):
    KeMsgProcess(peer_id),start_video_(false),start_audio_(false)
{
}


void KeMessageProcessCamera::OnMessageRespond(talk_base::Buffer &msgData)
{
    char msgType = msgData.data()[1];
    switch(msgType){
    case KEMSG_TYPE_VIDEOSERVER:
        RecvAskMediaMsg(msgData);
        break;
    default:
        KeMsgProcess::OnMessageRespond(msgData);
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
    }else{
        start_video_ = false;
    }


    int audio = msg->listen;
    if(audio == 0){
        start_audio_ = true;
    }else{
        start_audio_ = false;
    }

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



KeMessageProcessClient::KeMessageProcessClient(std::string peer_id):
    KeMsgProcess(peer_id)
{

}

void KeMessageProcessClient::AskVideo()
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
    pReqMsg->videoID = 0;
    pReqMsg->video = 0;
    pReqMsg->listen = 0;
    pReqMsg->talk = 0;
    pReqMsg->protocalType = 0;
    pReqMsg->transSvrIp = 0;

    SignalNeedSendData(this->peer_id_,sendBuf.data(),sendBuf.length());

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
        KERTStreamHead * pMsg = (KERTStreamHead *)msgData.data();
        const int sendStartPos = 11;
        int mediaDataLen = msgData.length() - sendStartPos;
        if(msgType == KEMSG_TYPE_VIDEOSTREAM){
            SignalRecvVideoData(this->peer_id_,msgData.data() + sendStartPos,mediaDataLen);
        }else if(msgType == KEMSG_TYPE_AUDIOSTREAM){
            SignalRecvAudioData(this->peer_id_,msgData.data() + sendStartPos,mediaDataLen);
        }
        break;
    }
    default:
        KeMsgProcess::OnMessageRespond(msgData);
        break;
    }

}
