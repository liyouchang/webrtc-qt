#include "ketunnelclient.h"

#include "talk/base/json.h"
#include "talk/base/logging.h"
#include "talk/base/stringutils.h"

#include <limits.h>

#include "KeMessage.h"
#include "defaults.h"
#include "recorderavi.h"
#include "recordermp4.h"
//#include "ke08recorder.h"

namespace kaerp2p {


KeTunnelClient::KeTunnelClient()
{
}

bool KeTunnelClient::SendCommand(const std::string &peer_id,
                                 const std::string &command)
{
    Json::Reader reader;
    Json::Value jmessage;
    if (!reader.parse(command, jmessage)) {
        LOG(WARNING) <<"command format error. " << command;
        return false;
    }
    jmessage[kKaerMsgTypeName] = kKaerTunnelMsgTypeValue;

    Json::StyledWriter writer;
    std::string msg = writer.write(jmessage);

    return this->terminal_->SendByRouter(peer_id,msg);
    return true;
}

bool KeTunnelClient::StartPeerMedia(std::string peer_id, int video)
{
    KeMessageProcessClient * process =
            dynamic_cast<KeMessageProcessClient *>( this->GetProcess(peer_id));
    if(process == NULL) {
        LOG(WARNING) << "process not found "<<peer_id;
        return false;
    }
//    process->AskVideo(0,0,0);
    process->AskVideo(video,1,0);
    return true;
}

bool KeTunnelClient::StopPeerMedia(std::string peer_id)
{
    KeMessageProcessClient * process =
            dynamic_cast<KeMessageProcessClient *>(this->GetProcess(peer_id));
    if (process == NULL) {
        LOG(WARNING) << "process not found "<<peer_id;
        return false;
    }
    process->AskVideo(0,0,0);
    process->StopVideoCut();
    return true;
}

bool KeTunnelClient::StartPeerTalk(std::string peer_id,bool withListen)
{
    KeMessageProcessClient * process =
            dynamic_cast<KeMessageProcessClient *>(this->GetProcess(peer_id));
    if (process == NULL) {
        LOG(WARNING) << "process not found "<<peer_id;
        return false;
    }
    if(withListen){//连同监听一起开启
        process->AskVideo(MEDIA_NOCHANGE,1,1);
    }else{
        process->AskVideo(MEDIA_NOCHANGE,MEDIA_NOCHANGE,1);
    }
//    this->SignalTalkData.connect(process,&KeMessageProcessClient::OnTalkData);
    return true;
}

bool KeTunnelClient::StopPeerTalk(std::string peer_id, bool withListen)
{
    KeMessageProcessClient * process =
            dynamic_cast<KeMessageProcessClient *>(this->GetProcess(peer_id));
    if (process == NULL) {
        LOG(WARNING) << "process not found "<<peer_id;
        return false;
    }
    if(withListen) {//连同监听一起关闭
        process->AskVideo(MEDIA_NOCHANGE,0,0);
    } else {//监听保持原状态
        process->AskVideo(MEDIA_NOCHANGE,MEDIA_NOCHANGE,0);
    }
    this->SignalTalkData.disconnect(process);
    return true;
}

bool KeTunnelClient::StartPeerVideoCut(const std::string & peer_id,
                                       const std::string &filename)
{
    KeMessageProcessClient * process =
            dynamic_cast<KeMessageProcessClient *>(this->GetProcess(peer_id));
    if(process == NULL){
        LOG(WARNING) <<"KeTunnelClient::PeerVideoCut---"<<
                       "process not found "<<peer_id;
        return false;
    }
    return process->StartVideoCut(filename);
}

bool KeTunnelClient::StopPeerVideoCut(const std::string &peer_id)
{
    KeMessageProcessClient * process =
            dynamic_cast<KeMessageProcessClient *>(this->GetProcess(peer_id));
    if(process == NULL){
        LOG(WARNING) <<"KeTunnelClient::PeerVideoCut---"<<
                       "process not found "<<peer_id;
        return false;
    }
    return process->StopVideoCut();
}

bool KeTunnelClient::PlayRemoteFile(std::string peerId,std::string remoteFileName)
{
    KeMessageProcessClient * process =
            dynamic_cast<KeMessageProcessClient *>(this->GetProcess(peerId));
    if(process == NULL){
        LOG(WARNING) << "process not found "<<peerId;
        return false;
    }
    return  process->ReqestPlayFile(remoteFileName.c_str());
}
/**
 * @brief KeTunnelClient::SetPlayFileStatus
 * @param peerId
 * @param jstrStatus {position:50,speed:0x10,type:3}
 * @return
 */
bool KeTunnelClient::SetPlayFileStatus(std::string peerId, int type,int position ,int speed)
{
    KeMessageProcessClient * process =
            dynamic_cast<KeMessageProcessClient *>(this->GetProcess(peerId));
    if(process == NULL){
        LOG(WARNING) <<"KeTunnelClient::PeerVideoCut---"<<
                       "process not found "<<peerId;
        return false;
    }
    process->SetPlayFileStatus(type,position,speed);
    return true;

}

void KeTunnelClient::OnTunnelOpened(PeerTerminalInterface *t,
                                    const std::string &peer_id)
{
    ASSERT(this->terminal_ == t);
    KeMessageProcessClient * process = new KeMessageProcessClient(peer_id,this);
    process->SignalRecvAudioData.connect(this,&KeTunnelClient::OnRecvAudioData);
    process->SignalRecvVideoData.connect(this,&KeTunnelClient::OnRecvVideoData);
    this->AddMsgProcess(process);
}

void KeTunnelClient::OnRouterMessage(const std::string &peer_id,
                                     talk_base::Buffer &msg)
{
    std::string strMsg(msg.data(),msg.length());
    Json::Reader reader;
    Json::Value jmessage;
    if (!reader.parse(strMsg, jmessage)) {
        LOG(WARNING) << "Received unknown message. " << strMsg;
        return;
    }
    std::string command;
    bool ret = GetStringFromJsonObject(jmessage, kKaerMsgCommandName, &command);
    if(!ret){
        LOG(WARNING)<<"get command error"<<peer_id<<" Msg "<<strMsg;
        return;
    }
}

void KeTunnelClient::SendTalkData(const char *data, int len)
{
    KEFrameHead frameHead;
    frameHead.frameNo = 0;
    frameHead.piecesNo = 0;
    //time span is not use
    int ams = talk_base::Time();
    frameHead.second = ams/1000;
    frameHead.millisecond = (ams%1000)/10;
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
    LOG(LS_VERBOSE)<<"KeTunnelClient::OnRecvAudioData";
}

void KeTunnelClient::OnRecvVideoData(const std::string &peer_id,
                                     const char *data, int len)
{
    LOG(LS_VERBOSE)<<"KeTunnelClient::OnRecvVideoData";
}

//void KeTunnelClient::OnRecordFileData(const std::string &peer_id,
//                                      const char *data, int len)
//{
//    LOG(INFO)<<"KeTunnelClient::OnRecordFileData";
//}

void KeTunnelClient::OnRecordStatus(const std::string &peer_id, int status, int position, int speed)
{
    LOG_F(INFO)<<peer_id<<" status "<<status;
}

void KeTunnelClient::OnMediaStatus(const std::string &peer_id, int video, int audio, int talk)
{
    LOG_F(INFO)<<" from "<<peer_id<<", video-"<<video<<", audio-"<<audio<<
                 ", talk-"<<talk;

}


KeMessageProcessClient::KeMessageProcessClient(std::string peer_id,
                                               KeTunnelClient * container):
    KeMsgProcess(peer_id,container),recordSaver(NULL),talk_status(0)
{

}

KeMessageProcessClient::~KeMessageProcessClient()
{
    StopVideoCut();
}
// -1 表示保持原状 0 表示关闭 >0 表示开启
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

bool KeMessageProcessClient::ReqestPlayFile(const char *remoteFile)
{
    //    LOG(INFO)<<"KeMessageProcessClient::ReqestPlayFile---"<<
    //               "shouldplaySize="<<playSize;
    talk_base::Buffer sendBuf;
    int msgLen = sizeof(KEPlayRecordFileReq);
    sendBuf.SetLength(msgLen);
    KEPlayRecordFileReq * pReqMsg;
    pReqMsg = (KEPlayRecordFileReq *)sendBuf.data();
    pReqMsg->protocal = PROTOCOL_HEAD;
    pReqMsg->msgType = KEMSG_REQUEST_PLAY_FILE;
    pReqMsg->msgLength = msgLen;
    pReqMsg->clientID = 0;
    pReqMsg->playSpeed = -1;
    pReqMsg->fileType = 1;
    talk_base::strcpyn(pReqMsg->fileData,80,remoteFile);
    SignalNeedSendData(this->peer_id(),sendBuf.data(),sendBuf.length());
    return true;
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


bool KeMessageProcessClient::StartVideoCut(const std::string &filename)
{
    if(recordSaver){
        LOG_F(INFO)<<"record already started";
        return false;
    }
    LOG_F(INFO)<<" start video cut frame rate "<<videoInfo_.frameRate<<" resolution "<<videoInfo_.frameResolution;
//    recordSaver = new RecorderAvi(this->peer_id(),videoInfo_.frameRate,
//                                  videoInfo_.frameResolution);

    int height,width;
    Reso2WidthHeigh(videoInfo_.frameResolution,&width,&height);
    recordSaver = new RecorderMp4(this->peer_id(),videoInfo_.frameRate,width,height);

    bool ret = recordSaver->StartSave(filename);
    this->SignalRecvVideoData.connect(recordSaver,&RecordSaverInterface::OnVideoData);
    this->SignalRecvAudioData.connect(recordSaver,&RecordSaverInterface::OnAudioData);
    if(!ret){
        delete recordSaver;
        recordSaver = NULL;
    }
    return ret;
}

bool KeMessageProcessClient::StopVideoCut()
{

    if(recordSaver == NULL){
        LOG(INFO)<<"KeMessageProcessClient::StopVideoCut---"<<
                   "video cut is not start";
        return false;
    }
    this->SignalRecvVideoData.disconnect(recordSaver);
    this->SignalRecvAudioData.disconnect(recordSaver);
    LOG_F(INFO)<<"stop save";
    bool ret = recordSaver->StopSave();
    delete recordSaver;
    recordSaver = NULL;
    return ret;
}

void KeMessageProcessClient::SetPlayFileStatus(int type,int position ,int speed)
{
    talk_base::Buffer sendBuf;
    int msgLen = sizeof(KEPlayRecordFileReq);
    sendBuf.SetLength(msgLen);
    KEPlayRecordFileReq * pReqMsg;
    pReqMsg = (KEPlayRecordFileReq *)sendBuf.data();
    pReqMsg->protocal = PROTOCOL_HEAD;
    pReqMsg->msgType = KEMSG_REQUEST_PLAY_FILE;
    pReqMsg->msgLength = msgLen;
    pReqMsg->playSpeed = speed;
    pReqMsg->playPos = position;
    pReqMsg->videoID = 0;
    pReqMsg->fileType = type;
    //    talk_base::strcpyn(pReqMsg->fileData,80,remoteFile);
    SignalNeedSendData(this->peer_id(),sendBuf.data(),sendBuf.length());
}

void KeMessageProcessClient::OnMessageRespond(talk_base::Buffer &msgData)
{
    char msgType = msgData.data()[1];
    switch(msgType)
    {
    case KEMSG_TYPE_VIDEOSERVER:
        RecvAskMediaResp(msgData);
        break;
    case KEMSG_TYPE_MEDIATRANS:
        break;
    case KEMSG_TYPE_AUDIOSTREAM:{
        RecvAudioData(msgData);
        break;
    }
    case KEMSG_TYPE_VIDEOSTREAM:{
        RecvVideoData(msgData);
        break;
    }
    case KEMSG_REQUEST_PLAY_FILE:{
        RecvPlayFileResp(msgData);
        break;
    }
    case KEMSG_RecordPlayData:{
        OnRecvRecordMsg(msgData);
        break;
    }
    case KEMSG_TYPE_MEDIASTATUS:{
        this->RecvMediaStatus(msgData);
        break;
    }
    default:
        KeMsgProcess::OnMessageRespond(msgData);
        break;
    }

}

void KeMessageProcessClient::RecvVideoData(talk_base::Buffer &msgData)
{
    const int sendStartPos = 11;
    int mediaDataLen = msgData.length() - sendStartPos;
    SignalRecvVideoData(this->peer_id(),msgData.data() +
                        sendStartPos,mediaDataLen);

}

void KeMessageProcessClient::RecvAudioData(talk_base::Buffer &msgData)
{
    const int sendStartPos = 11;
    int mediaDataLen = msgData.length() - sendStartPos;
    SignalRecvAudioData(this->peer_id(),msgData.data() +
                        sendStartPos,mediaDataLen);
}

void KeMessageProcessClient::OnRecvRecordMsg(talk_base::Buffer &msgData)
{
    KEPlayRecordDataHead * phead = (KEPlayRecordDataHead *)msgData.data();

    KeTunnelClient *client  = static_cast<KeTunnelClient *>(container_);

    if(phead->resp == RESP_ACK){
//        const int data_pos = sizeof(KEPlayRecordDataHead);
//        int mediaDataLen = msgData.length() - data_pos;

        //        SignalRecvVideoData(this->peer_id(),msgData.data() +
        //                            sendStartPos,mediaDataLen);
    }
    else if(phead->resp == RESP_END){
        //        LOG(INFO)<<"KeMessageProcessClient::OnRecvRecordMsg---end";
        //        client->OnRecordStatus(this->peer_id(),kDownloadEnd);
        //        recordSaver->StopSave();
        //        delete recordSaver;
        //        recordSaver = NULL;
    }
}


void KeMessageProcessClient::RecvAskMediaResp(talk_base::Buffer &msgData)
{
    KEVideoServerResp * msg = (KEVideoServerResp *)msgData.data();
    this->videoInfo_.frameRate = msg->frameRate;
    this->videoInfo_.frameResolution = msg->frameType;
    LOG(INFO)<<"KeMessageProcessClient::RecvAskMediaResp---"<<
               msg->frameRate<<msg->frameType;
}

void KeMessageProcessClient::RecvPlayFileResp(talk_base::Buffer &msgData)
{
    KeTunnelClient *client  = static_cast< KeTunnelClient *>(container_);
    KEPlayRecordFileResp *msg = (KEPlayRecordFileResp*)msgData.data();
    if(msg->resp == RESP_ACK){
        LOG_F(INFO)<<"start remote play, frateRate="<<msg->frameRate<<
                     ", framteResolution="<<msg->frameResolution;
        this->videoInfo_.frameRate = msg->frameRate;
        this->videoInfo_.frameResolution = msg->frameResolution;
        client->OnRecordStatus(this->peer_id(),kRecordStartPlay,msg->playPos,msg->playSpeed);
        return;
    }else if(msg->resp == RESP_NAK){
        LOG_F(WARNING)<<"file error";
        client->OnRecordStatus(this->peer_id(),kRequestFileError,MEDIA_NOCHANGE,MEDIA_NOCHANGE);
    }else if(msg->resp == RESP_CTRL){
        client->OnRecordStatus(this->peer_id(),kRecordPlaying,msg->playPos,msg->playSpeed);
    }else if(msg->resp == RESP_END){
        client->OnRecordStatus(this->peer_id(),kRecordPlayEnd,msg->playPos,msg->playSpeed);
    }
    else{
        LOG_F(WARNING)<<"message error";
        client->OnRecordStatus(this->peer_id(),kRequestMsgError,MEDIA_NOCHANGE,MEDIA_NOCHANGE);
    }
    //    recordSaver->StopSave();
    //    delete recordSaver;
    //    recordSaver = NULL;
}

void KeMessageProcessClient::RecvMediaStatus(talk_base::Buffer &msgData)
{
    KeTunnelClient *client  = static_cast< KeTunnelClient *>(container_);

    KEMediaStatus * msg = (KEMediaStatus *)msgData.data();

    if( talk_status != msg->talk && msg->talk != MEDIA_NOCHANGE){
        LOG_F(INFO)<<"media status: video-"<<msg->video<<" audio-"<<msg->listen<<" talk-"<<msg->talk;
        talk_status = msg->talk;
        if(talk_status == 1){
            client->SignalTalkData.connect(this,&KeMessageProcessClient::OnTalkData);
        }else if(talk_status == 0){
            client->SignalTalkData.disconnect(this);
        }
        client->OnMediaStatus(this->peer_id(),msg->video,msg->listen,msg->talk);
    }

}

}
