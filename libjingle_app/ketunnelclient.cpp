#include "ketunnelclient.h"

#include "talk/base/json.h"
#include "talk/base/logging.h"
#include "talk/base/stringutils.h"

#include "KeMessage.h"
#include "defaults.h"

KeTunnelClient::KeTunnelClient()
{

}

int KeTunnelClient::SendCommand(const std::string &peer_id,
                                const std::string &command)
{
    Json::Reader reader;
    Json::Value jmessage;
    if (!reader.parse(command, jmessage)) {
        LOG(WARNING) << "command format error. " << command;
        return 101;
    }
    jmessage[kKaerMsgTypeName] = kKaerTunnelMsgTypeValue;

    Json::StyledWriter writer;
    std::string msg = writer.write(jmessage);

    return this->terminal_->SendByRouter(peer_id,msg);
}

int KeTunnelClient::StartPeerMedia(std::string peer_id, int video)
{
    KeMessageProcessClient * process =
            dynamic_cast<KeMessageProcessClient *>( this->GetProcess(peer_id));
    if(process == NULL){
        LOG(WARNING) << "process not found "<<peer_id;
        return -1;
    }
    if(video == 0){
        process->AskVideo(0,0,0);
    }
    else{
        process->AskVideo(0,0,0);
        process->AskVideo(video,1,1);
    }
    return 0;
}

bool KeTunnelClient::DownloadRemoteFile(std::string peer_id,
                                        std::string remote_file_name)
{
    KeMessageProcessClient * process =
            dynamic_cast<KeMessageProcessClient *>(this->GetProcess(peer_id));
    if(process == NULL){
        LOG(WARNING) << "process not found "<<peer_id;
        return false;
    }

    process->ReqestPlayFile(remote_file_name.c_str());

    return true;
}



void KeTunnelClient::OnTunnelOpened(PeerTerminalInterface *t,
                                    const std::string &peer_id)
{
    ASSERT(this->terminal_ == t);
    KeMessageProcessClient * process = new KeMessageProcessClient(peer_id,this);
    process->SignalRecvAudioData.connect(this,&KeTunnelClient::OnRecvAudioData);
    process->SignalRecvVideoData.connect(this,&KeTunnelClient::OnRecvVideoData);
    this->SignalTalkData.connect(process,&KeMessageProcessClient::OnTalkData);
    this->AddMsgProcess(process);
}

void KeTunnelClient::OnRouterMessage(const std::string &peer_id,
                                     const std::string &msg)
{
    Json::Reader reader;
    Json::Value jmessage;
    if (!reader.parse(msg, jmessage)) {
        LOG(WARNING) << "Received unknown message. " << msg;
        return;
    }
    std::string command;
    bool ret = GetStringFromJsonObject(jmessage, kKaerMsgCommandName, &command);
    if(!ret){
        LOG(WARNING)<<"get command error"<<peer_id;
        return;
    }

}

void KeTunnelClient::SendTalkData(const char *data, int len)
{
    KEFrameHead frameHead;
    frameHead.frameNo = 0;
    frameHead.piecesNo = 0;
    //time span is not use
    //int ams = talk_base::Time();
    //frameHead.second = ams/1000;
    //frameHead.millisecond = (ams%1000)/10;
    //frame type: is not use
    frameHead.frameType = 80;
    frameHead.frameLen = len;
    talk_base::Buffer frameBuf(&frameHead,sizeof(KEFrameHead));
    frameBuf.AppendData(data,len);
    SignalTalkData(frameBuf.data(),frameBuf.length());

}

void KeTunnelClient::OnRecvAudioData(const std::string &peer_id,
                                     const char *data, int len)
{
    LOG(INFO)<<__FUNCTION__;
}

void KeTunnelClient::OnRecvVideoData(const std::string &peer_id,
                                     const char *data, int len)
{
    LOG(INFO)<<__FUNCTION__;
}

void KeTunnelClient::OnRecordFileData(const std::string &peer_id,
                                      const char *data, int len)
{
    LOG(INFO)<<"KeTunnelClient::OnRecordFileData";
}

void KeTunnelClient::OnRecordStatus(const std::string &peer_id, int status)
{
    LOG(INFO)<<"KeTunnelClient::OnRecordStatus---"<<peer_id<<" status "<<status;

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


