#include "ketunnelcamera.h"

#include "talk/base/json.h"
#include "talk/base/logging.h"
#include "talk/base/timeutils.h"
#include "talk/base/stringutils.h"
#include "KeMessage.h"
#include "defaults.h"
#include "recorderavi.h"

namespace kaerp2p{


void KeTunnelCamera::OnTunnelOpened(PeerTerminalInterface *t,
                                    const std::string &peer_id)
{
    ASSERT(terminal_ == t);
    LOG(INFO)<<__FUNCTION__<<"---------"<<peer_id;
    KeMessageProcessCamera *process = new KeMessageProcessCamera(peer_id,this);
    this->AddMsgProcess(process);
}

void KeTunnelCamera::SetVideoClarity(int)
{
}

int KeTunnelCamera::GetVideoClarity()
{
    return 2;
}

void KeTunnelCamera::OnRecvRecordQuery( std::string peer_id,
                                        std::string condition)
{
    LOG(INFO)<<"KeTunnelCamera::OnRecvRecordQuery---"
            <<peer_id<<" query:"<<condition ;
}

void KeTunnelCamera::SetPtz(std::string ptz_key, int param)
{
    LOG(INFO)<<"KeTunnelCamera::SetPtz---key:" <<ptz_key<<" param:"<<param;
}

void KeTunnelCamera::RecvGetWifiInfo(std::string peer_id)
{
    LOG(INFO)<<"KeTunnelCamera::OnRecvGetWifiInfo---from:" <<peer_id;
}

void KeTunnelCamera::SetWifiInfo(std::string peer_id, std::string param)
{
    LOG(INFO)<<"KeTunnelCamera::SetWifiInfo---from:"
            <<peer_id<<" param:"<<param ;
}

void KeTunnelCamera::OnToPlayFile(const std::string &peer_id,
                                  const std::string &filename)
{
    LOG(INFO) << "KeTunnelCamera::OnToPlayFile --- "
              <<peer_id<<" file name "<<filename ;
}

void KeTunnelCamera::OnRecvTalkData(const std::string &peer_id, const char *data, int len)
{
}

void KeTunnelCamera::OnRouterMessage(const std::string &peer_id,
                                     const std::string &msg)
{
    LOG(INFO)<<"KeTunnelCamera::OnRouterMessage---" << msg;
    Json::Reader reader;
    Json::Value jmessage;
    if (!reader.parse(msg, jmessage)) {
        LOG(WARNING) << "Received unknown message. " << msg;
        return;
    }
    std::string command;
    bool ret = GetStringFromJsonObject(jmessage, kKaerMsgCommandName, &command);
    if(!ret){
        LOG(WARNING)<<"get command error-"<<command<<" from"<<peer_id ;
        return;
    }
    if(command.compare("video_clarity") == 0){
        int clarity;
        GetIntFromJsonObject(jmessage,"value",&clarity);
        OnRecvVideoClarity(peer_id,clarity);
    }else if(command.compare("ptz") == 0){
        std::string ptz_control;
        GetStringFromJsonObject(jmessage,"control",&ptz_control);
        int param;
        GetIntFromJsonObject(jmessage,"param",&param);
        std::string ptz_key = "ptz_";
        ptz_key += ptz_control;
        this->SetPtz(ptz_key,param);
    }else if(command.compare("query_record") == 0){
        Json::Value jcondition;
        if(!GetValueFromJsonObject(jmessage, "condition", &jcondition)){
            LOG(WARNING)<<"get param value error from"<<
                          peer_id<< " msg"<<msg;
            return;
        }
        std::string condition  = JsonValueToString(jcondition);
        OnRecvRecordQuery(peer_id,condition);
    }else if(command.compare("echo") == 0){
        this->terminal_->SendByRouter(peer_id,msg);
    }else if(command.compare("wifi_info") == 0){
        this->RecvGetWifiInfo(peer_id);
    }else if(command.compare("set_wifi") == 0){
        Json::Value jwifiParam;
        if(!GetValueFromJsonObject(jmessage, "param", &jwifiParam)){
            LOG(WARNING)<<"get param value error from"<<
                          peer_id<< " msg"<<msg;
            return;
        }
        std::string paramStr  = JsonValueToString(jwifiParam);
        this->SetWifiInfo(peer_id,paramStr);
    }
    else{
        LOG(WARNING)<<"receive unexpected command from "<<
                      peer_id<< " msg"<<msg;
    }
}

void KeTunnelCamera::OnRecvVideoClarity(std::string peer_id, int clarity)
{
    LOG(INFO)<<"KeTunnelCamera::OnRecvVideoClarity---"
            <<peer_id<<" clarity:"<<clarity ;
    if(clarity == 101){
        int clarity = this->GetVideoClarity();
        Json::Value jmessage;
        jmessage[kKaerMsgTypeName] = kKaerTunnelMsgTypeValue;
        jmessage[kKaerMsgCommandName] = "video_clarity";
        jmessage["value"] = clarity;
        Json::StyledWriter writer;
        std::string msg = writer.write(jmessage);
        this->terminal_->SendByRouter(peer_id,msg);
    }else{
        this->SetVideoClarity(clarity);
    }
}


KeMessageProcessCamera::KeMessageProcessCamera(std::string peer_id,
                                               KeTunnelCamera *container):
    KeMsgProcess(peer_id,container),video_started_(false),audio_started_(false),
    talk_started_(false),recordReader(NULL)
{
}

KeMessageProcessCamera::~KeMessageProcessCamera()
{
    if(recordReader != NULL){
        recordReader->StopRead();
        delete recordReader;
        recordReader = NULL;
    }
}


void KeMessageProcessCamera::OnMessageRespond(talk_base::Buffer &msgData)
{
    char msgType = msgData.data()[1];
    switch(msgType){
    case KEMSG_TYPE_VIDEOSERVER:
        RecvAskMediaReq(msgData);
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

void KeMessageProcessCamera::RecvAskMediaReq(talk_base::Buffer &msgData)
{
    //send stream
    KEVideoServerReq * msg = (KEVideoServerReq *)msgData.data();
    LOG(INFO)<< "KeMessageProcessCamera::RecvAskMediaMsg---"
             <<"receive ask media msg : video-"<<msg->video<<
               " listen-"<<msg->listen<<" talk-"<<msg->talk;
    ConnectMedia(msg->video,msg->listen,msg->talk);
}

void KeMessageProcessCamera::RecvPlayFile(talk_base::Buffer &msgData)
{
    KEPlayRecordFileReq * pMsg =
            reinterpret_cast<KEPlayRecordFileReq *>(msgData.data());
    LOG(INFO)<< "KeMessageProcessCamera::RecvPlayFile--"<<pMsg->fileData;
    std::string fileName = pMsg->fileData;
    if(recordReader != NULL){
        LOG(INFO)<<"KeMessageProcessCamera::RecvPlayFile---"<<
                   "already start record read";
        RespPlayFileReq(RESP_NAK,fileName.c_str());
        return;
    }
    recordReader = new RecordReaderAvi(1);
    if(!recordReader->StartRead(fileName)){
        LOG(INFO)<<"KeMessageProcessCamera::RecvPlayFile---"<<
                   "start read failed";
        delete recordReader;
        recordReader = NULL;
        RespPlayFileReq(RESP_NAK,fileName.c_str());
        return;
    }
    recordReader->SignalAudioData.connect(this,&KeMessageProcessCamera::OnRecordData);
    recordReader->SignalVideoData.connect(this,&KeMessageProcessCamera::OnRecordData);
    recordReader->SignalRecordEnd.connect(this,&KeMessageProcessCamera::OnRecordReadEnd);
    RespPlayFileReq(RESP_ACK,fileName.c_str());
}

void KeMessageProcessCamera::RecvTalkData(talk_base::Buffer &msgData)
{
    KERTStreamHead * pMsg = (KERTStreamHead *)msgData.data();
    const int sendStartPos = 11;
    int mediaDataLen = msgData.length() - sendStartPos;
    if( pMsg->msgType == KEMSG_TYPE_AUDIOSTREAM ) {
        SignalRecvTalkData(this->peer_id(),msgData.data() +
                           sendStartPos,mediaDataLen);
    } else {
        LOG(WARNING)<<"KeMessageProcessCamera::RecvTalkData---"<<
                      "message type error";
    }
}

void KeMessageProcessCamera::RespAskMediaReq(const VideoInfo &info)
{
    talk_base::Buffer sendBuf;
    int msgLen = sizeof(KEVideoServerResp);
    sendBuf.SetLength(msgLen);
    KEVideoServerResp * msg = (KEVideoServerResp *)sendBuf.data();
    msg->protocal = PROTOCOL_HEAD;
    msg->msgType = KEMSG_TYPE_VIDEOSERVER;
    msg->msgLength = msgLen;
    msg->videoID = 0;
    msg->channelNo = 1;
    msg->respType = RESP_ACK;
    msg->frameRate = info.frameRate;
    msg->frameType = info.frameResolution;
    SignalNeedSendData(this->peer_id(),sendBuf.data(),sendBuf.length());
}

void KeMessageProcessCamera::RespPlayFileReq(int resp,const char * fileName)
{
    talk_base::Buffer sendBuf;
    int msgLen = sizeof(KEPlayRecordFileResp);
    sendBuf.SetLength(msgLen);
    KEPlayRecordFileResp * msg = (KEPlayRecordFileResp *)sendBuf.data();
    msg->protocal = PROTOCOL_HEAD;
    msg->msgType = KEMSG_REQUEST_PLAY_FILE;
    msg->msgLength = msgLen;
    msg->videoID = 0;
    msg->channelNo = 1;
    msg->resp = resp;
    msg->frameRate = recordReader->recordInfo.frameRate;
    msg->frameResolution = recordReader->recordInfo.frameResolution;
    talk_base::strcpyn(msg->fileName,80,fileName);
    SignalNeedSendData(this->peer_id(),sendBuf.data(),sendBuf.length());
}

void KeMessageProcessCamera::ConnectMedia(int video, int audio, int talk)
{
    KeTunnelCamera * camera = static_cast<KeTunnelCamera *>(container_);
    camera->GetCameraVideoInfo(video,&this->videoInfo_);
    if(video == 0){//stop
        camera->SignalVideoData1.disconnect(this);
        camera->SignalVideoData2.disconnect(this);
        this->video_started_ = false;
    }
    else if(!video_started_){
        this->RespAskMediaReq(this->videoInfo_);
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
    if(audio == 0){
        camera->SignalAudioData.disconnect(this);
        this->audio_started_ = false;
    }
    else if(!audio_started_){
        audio_started_ = true;
        camera->SignalAudioData.connect(
                    this,&KeMessageProcessCamera::OnAudioData);
    }
    if(talk == 0){
        this->SignalRecvTalkData.disconnect(camera);
        this->talk_started_ = false;
    }else if(!talk_started_){
        this->SignalRecvTalkData.connect(camera,&KeTunnelCamera::OnRecvTalkData);
        this->talk_started_ = true;
    }
}

void KeMessageProcessCamera::OnVideoData(const char *data, int len)
{
    talk_base::Buffer sendBuf;
    int msgLen = sizeof(KERTStreamHead) +sizeof(KEFrameHead) + len;
    KERTStreamHead  streamHead;
    streamHead.protocal = PROTOCOL_HEAD;
    streamHead.msgType = KEMSG_TYPE_VIDEOSTREAM;
    streamHead.msgLength = msgLen;
    streamHead.channelNo = 1;
    streamHead.videoID = 0;
    sendBuf.AppendData(&streamHead,sizeof(KERTStreamHead));
    static unsigned short frameNo = 0;
    KEFrameHead frameHead;
    frameHead.frameNo = frameNo++;
    frameHead.piecesNo = 1;
    //time set
    int ams = talk_base::Time();
    frameHead.second = ams/1000;
    frameHead.millisecond = (ams%1000)/10;
    frameHead.frameType = this->videoInfo_.frameResolution;
    frameHead.frameLen = len;
    sendBuf.AppendData(&frameHead,sizeof(KEFrameHead));
    sendBuf.AppendData(data,len);
    SignalNeedSendData(this->peer_id(),sendBuf.data(),sendBuf.length());
}

void KeMessageProcessCamera::OnAudioData(const char *data, int len)
{
    const int kNalHeadLen = 4;
    const char kNalhead[kNalHeadLen] = {0,0,0,1};
    talk_base::Buffer sendBuf;
    int msgLen = sizeof(KERTStreamHead)+sizeof(KEFrameHead)+kNalHeadLen+len;
    KERTStreamHead streamHead;
    streamHead.protocal = PROTOCOL_HEAD;
    streamHead.msgType = KEMSG_TYPE_AUDIOSTREAM;
    streamHead.msgLength = msgLen;
    streamHead.channelNo = 1;
    streamHead.videoID = 0;
    sendBuf.AppendData(&streamHead,sizeof(KERTStreamHead));
    KEFrameHead frameHead;
    frameHead.frameNo = 0;
    frameHead.piecesNo = 0;
    //time set
    int ams = talk_base::Time();
    frameHead.second = ams/1000;
    frameHead.millisecond = (ams%1000)/10;
    frameHead.frameType = 80;//audio type
    frameHead.frameLen = len+4;
    sendBuf.AppendData(&frameHead,sizeof(KEFrameHead));
    sendBuf.AppendData(kNalhead,kNalHeadLen);
    sendBuf.AppendData(data,len);
    SignalNeedSendData(this->peer_id(),sendBuf.data(),sendBuf.length());
}

void KeMessageProcessCamera::OnRecordData(const char *data, int len)
{
    const int kNalHeadLen = 4;
    const char kNalhead[kNalHeadLen] = {0,0,0,1};
    int frameType  = 80;
    int msgLen = sizeof(KEPlayRecordDataHead)+sizeof(KEFrameHead)+kNalHeadLen+len;
    if(data[0]==kNalhead[0] && data[1] ==kNalhead[1] &&
            data[2] == kNalhead[2] && data[3] == kNalhead[3]){ // video data
        frameType = this->recordReader->recordInfo.frameResolution;
        msgLen = sizeof(KEPlayRecordDataHead)+sizeof(KEFrameHead)+len;
    }
    talk_base::Buffer sendBuf;
    KEPlayRecordDataHead streamHead;
    streamHead.protocal = PROTOCOL_HEAD;
    streamHead.msgType = KEMSG_RecordPlayData;
    streamHead.msgLength = msgLen;
    streamHead.channelNo = 1;
    streamHead.videoID = 0;
    streamHead.resp = RESP_ACK;
    sendBuf.AppendData(&streamHead,sizeof(KEPlayRecordDataHead));
    KEFrameHead frameHead;
    int ams = talk_base::Time();
    frameHead.second = ams/1000;
    frameHead.millisecond = (ams%1000)/10;
    frameHead.frameType = frameType;
    if(frameType == 80){
        frameHead.frameNo = 0;
        frameHead.piecesNo = 0;
        frameHead.frameLen = len+4;
        sendBuf.AppendData(&frameHead,sizeof(KEFrameHead));
        sendBuf.AppendData(kNalhead,kNalHeadLen);
    }else{
        static unsigned short frameNo = 0;
        frameHead.frameNo = frameNo++;
        frameHead.piecesNo = 1;
        frameHead.frameLen = len;
        sendBuf.AppendData(&frameHead,sizeof(KEFrameHead));
    }
    sendBuf.AppendData(data,len);
    SignalNeedSendData(this->peer_id(),sendBuf.data(),sendBuf.length());
}

void KeMessageProcessCamera::OnRecordReadEnd(RecordReaderInterface *reader)
{
    ASSERT(recordReader == reader);
    recordReader->StopRead();
    delete recordReader;
    recordReader = NULL;
    int msgLen = sizeof(KEPlayRecordDataHead);
    talk_base::Buffer sendBuf;
    KEPlayRecordDataHead streamHead;
    streamHead.protocal = PROTOCOL_HEAD;
    streamHead.msgType = KEMSG_RecordPlayData;
    streamHead.msgLength = msgLen;
    streamHead.channelNo = 1;
    streamHead.videoID = 0;
    streamHead.resp = RESP_END;
    sendBuf.AppendData(&streamHead,sizeof(KEPlayRecordDataHead));
    SignalNeedSendData(this->peer_id(),sendBuf.data(),sendBuf.length());
}

}
