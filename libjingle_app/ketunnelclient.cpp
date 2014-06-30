#include "ketunnelclient.h"

#include "talk/base/json.h"
#include "talk/base/logging.h"
#include "talk/base/stringutils.h"

#include <limits.h>

#include "KeMessage.h"
#include "defaults.h"
#include "recorderavi.h"
#include "ke08recorder.h"

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
    if(process == NULL){
        LOG(WARNING) << "process not found "<<peer_id;
        return false;
    }
    process->AskVideo(0,0,0);
    process->AskVideo(video,1,1);
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

bool KeTunnelClient::DownloadRemoteFile(std::string peerId,
                                        std::string remoteFileName,
                                        std::string saveFileName,int playSize)
{
    KeMessageProcessClient * process =
            dynamic_cast<KeMessageProcessClient *>(this->GetProcess(peerId));
    if(process == NULL){
        LOG(WARNING) << "process not found "<<peerId;
        return false;
    }

    return  process->ReqestPlayFile(
                remoteFileName.c_str(),saveFileName.c_str(),playSize);
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
    LOG(LS_VERBOSE)<<"KeTunnelClient::OnRecvAudioData";
}

void KeTunnelClient::OnRecvVideoData(const std::string &peer_id,
                                     const char *data, int len)
{
    LOG(LS_VERBOSE)<<"KeTunnelClient::OnRecvVideoData";
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
    KeMsgProcess(peer_id,container),recordSaver(NULL),shouldPlaySize(INT_MAX)
{

}

KeMessageProcessClient::~KeMessageProcessClient()
{
    StopVideoCut();
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

bool KeMessageProcessClient::ReqestPlayFile(const char *remoteFile,
                                            const char *saveFile,int playSize)
{
    LOG(INFO)<<"KeMessageProcessClient::ReqestPlayFile---"<<
               "shouldplaySize="<<playSize;
    if(recordSaver){
        LOG(WARNING)<<"KeMessageProcessClient::ReqestPlayFile---"<<
                      "already start record";
        return false;
    }
    recordSaver = new Ke08RecordSaver();
    if(!recordSaver->StartSave(saveFile)){
        LOG(WARNING)<<"KeMessageProcessClient::ReqestPlayFile---"<<
                      "file start save error";
        return false;
    }
    shouldPlaySize = playSize;

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
        LOG(INFO)<<"KeMessageProcessClient::StartVideoCut---"<<
                   "record already started";
        return false;
    }
    recordSaver = new RecorderAvi(this->peer_id(),videoInfo_.frameRate,
                                  videoInfo_.frameResolution);
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
    bool ret = recordSaver->StopSave();
    delete recordSaver;
    recordSaver = NULL;
    return ret;
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

    KeTunnelClient *client  = static_cast< KeTunnelClient *>(container_);

    if(phead->resp == RESP_ACK){
        const int data_pos = sizeof(KEPlayRecordDataHead);
        int mediaDataLen = msgData.length() - data_pos;
        recordSaver->OnVideoData(
                    this->peer_id(),msgData.data()+data_pos,mediaDataLen);

        if(recordSaver->savedSize > this->shouldPlaySize){
            shouldPlaySize = INT_MAX;
            client->OnRecordStatus(this->peer_id(),kShouldPlay);

        }
    }
    else if(phead->resp == RESP_END){
        LOG(INFO)<<"KeMessageProcessClient::OnRecvRecordMsg---end";
        client->OnRecordStatus(this->peer_id(),kDownloadEnd);
        recordSaver->StopSave();
        delete recordSaver;
        recordSaver = NULL;
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
    LOG(INFO)<<"KeMessageProcessClient::RecvPlayFileResp---resp="<<msg->resp<<
               ",frateRate="<<msg->frameRate<<",framteResolution="<<
               msg->frameResolution;
    if(msg->resp == RESP_ACK){
        LOG(INFO)<<"KeMessageProcessClient::RecvPlayFileResp---"<<
                   "start playback";
        if(!recordSaver){
            LOG(WARNING)<<"record saver not init";
            client->OnRecordStatus(this->peer_id(),kRecordSaverError);
            return;
        }
        client->OnRecordStatus(this->peer_id(),kRequestSuccess);
        return;
    }else if(msg->resp == RESP_NAK){
        LOG(WARNING)<<"KeMessageProcessClient::RecvPlayFileResp---"<<
                      "file error";
        client->OnRecordStatus(this->peer_id(),kRequestFileError);
    }else{
        LOG(WARNING)<<"KeMessageProcessClient::RecvPlayFileResp---"<<
                      "message error";
        client->OnRecordStatus(this->peer_id(),kRequestMsgError);
    }
    recordSaver->StopSave();
    delete recordSaver;
    recordSaver = NULL;
}

}
