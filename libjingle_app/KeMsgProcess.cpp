#include "KeMsgProcess.h"
#include "talk/base/stream.h"
#include "KeMessage.h"

KeMsgProcess::KeMsgProcess(std::string peer_id):
    peer_id_(peer_id),msgMaxLen(8192)
{
    bufPos = 0;
    toRead = 0;
}

void KeMsgProcess::OnTunnelMessage(const std::string &peer_id, talk_base::Buffer &msg)
{
    ASSERT(this->peer_id_ == peer_id);
    ASSERT(msg.length() > 0);

    this->ExtractMessage(msg);
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
        if (msgRecv.length() == 0)//上一个消息已经读取完成
        {
            result = buffer.Read(headBuf+bufPos,headLen-bufPos,&read_bytes,&error);
            if(result == talk_base::SR_EOS){
                break;
            }
            if(read_bytes < headLen-bufPos)//消息头在最后几个字节，记录读取的字节，下次继续读取。
            {
                LOG(INFO)<<"Continue Read head in new package\r\n ";
                bufPos = read_bytes;
                break;
            }
            unsigned char  protocal = headBuf[0];
            //int msgLen= *((int*)&headBuf[2]); //arm will failed with this
            int msgLen;
            memcpy(&msgLen,&headBuf[2],4);
            if (protocal != PROTOCOL_HEAD ||  msgLen > msgMaxLen)
            {
                LOG(WARNING)<<"The message Protocal Head "<< read_bytes <<" error, msg len "
                             <<msgLen<<" ,Clear the recv buffer!\r\n";
                msgRecv.SetLength(0);
                break;
            }
            msgRecv.SetLength(msgLen);
            bufPos = 0;
            memcpy(msgRecv.data(),headBuf,headLen);
            bufPos += headLen;
            toRead =  msgLen-headLen;
            if (toRead != 0)//防止 headLen 越界
            {
                result = buffer.Read(msgRecv.data()+bufPos,toRead,&read_bytes,&error);
                if(result == talk_base::SR_EOS){
                    break;
                }
                bufPos += read_bytes;
                toRead -= read_bytes;
            }
        }
        else//上一个消息未完成读取
        {
            result = buffer.Read(msgRecv.data()+bufPos,toRead,&read_bytes,&error);
            if(result == talk_base::SR_EOS){
                break;
            }
            if (read_bytes < toRead){
                LOG(INFO)<<"to read more and more!";
            }
            bufPos += read_bytes;
            toRead -= read_bytes;
        }
        if(toRead == 0 && bufPos == msgRecv.length())//全部读取
        {
            this->OnMessageRespond(msgRecv);
            msgRecv.SetLength(0);
            bufPos = 0;
        }
    }

}

void KeMsgProcess::OnMessageRespond(talk_base::Buffer &msgData)
{
    LOG(INFO)<<__FUNCTION__;

}


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
        LOG(INFO)<<"Receive unkown message: "<<msgType;
        break;
    }

}
