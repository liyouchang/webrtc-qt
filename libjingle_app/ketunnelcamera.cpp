#include "ketunnelcamera.h"

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
    LOG(INFO)<<"KeTunnelCamera::OnTunnelOpened"<<"---"<<peer_id;
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

void KeTunnelCamera::OnToPlayFile(const std::string &peer_id,
                                  const std::string &filename)
{
    LOG(INFO) << "KeTunnelCamera::OnToPlayFile --- "
              <<peer_id<<" file name "<<filename ;
}

void KeTunnelCamera::OnRecvTalkData(const std::string &peer_id,
                                    const char *data, int len)
{
}

void KeTunnelCamera::OnCommandJsonMsg(const std::string &peerId, Json::Value &jmessage)
{
    std::string strMsg = JsonValueToString(jmessage);
    std::string command;
    bool ret = GetStringFromJsonObject(jmessage, kKaerMsgCommandName, &command);
    if(!ret){
        LOG(WARNING)<<"get command error-"<<command<<" from"<<peerId ;
        return;
    }
   if(command.compare("echo") == 0){
        LOG_F(INFO)<<"receive echo command "<<peerId<< " msg"<<strMsg;

        this->terminal_->SendByRouter(peerId,strMsg);
    }
    else{
        LOG(WARNING)<<"receive unexpected command from "<<
                      peerId<< " msg"<<strMsg;
    }
}

void KeTunnelCamera::ReportResult(const std::string &peerId, const std::string &command, bool result)
{
    Json::Value jmessage;
    jmessage["type"] = "tunnel";
    jmessage["command"] = command;
    jmessage["result"] = result;
    Json::StyledWriter writer;
    std::string msg = writer.write(jmessage);
    this->terminal_->SendByRouter(peerId,msg);

}

void KeTunnelCamera::ReportJsonMsg(const std::string &peerId, Json::Value &jmessage)
{
    Json::StyledWriter writer;
    std::string msg = writer.write(jmessage);
    LOG(LS_VERBOSE)<<"send msg is "<< msg;
    this->terminal_->SendByRouter(peerId,msg);

}

void KeTunnelCamera::OnRouterMessage(const std::string &peer_id,
                                     talk_base::Buffer &msg)
{
    std::string strMsg(msg.data(),msg.length());
    Json::Reader reader;
//    LOG_T(INFO)<<"KeTunnelCamera::OnRouterMessage---"<<peer_id << " msg "<<msg.data();
    Json::Value jmessage;
    if (!reader.parse(strMsg, jmessage)) {
        LOG(WARNING) << "Received unknown message. ";
        return;
    }

    this->OnCommandJsonMsg(peer_id,jmessage);
}

void KeTunnelCamera::OnRecvVideoClarity(std::string peer_id, int clarity)
{
    LOG(INFO)<<"KeTunnelCamera::OnRecvVideoClarity---"
            <<peer_id<<" clarity:"<<clarity ;
    if(clarity == 101)
    {
        int clarity = this->GetVideoClarity();
        Json::Value jmessage;
        jmessage[kKaerMsgTypeName] = kKaerTunnelMsgTypeValue;
        jmessage[kKaerMsgCommandName] = "video_clarity";
        jmessage["value"] = clarity;
        Json::StyledWriter writer;
        std::string msg = writer.write(jmessage);
        this->terminal_->SendByRouter(peer_id,msg);
    }else
    {
        this->SetVideoClarity(clarity);
    }
}


KeMessageProcessCamera::KeMessageProcessCamera(std::string peer_id,
                                               KeTunnelCamera *container):
    KeMsgProcess(peer_id,container),video_status(0),audio_status(0),
    talk_status(false),recordReader(NULL)
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
    ConnectMedia(msg->video,msg->listen,msg->talk);
}

void KeMessageProcessCamera::RecvPlayFile(talk_base::Buffer &msgData)
{
    KEPlayRecordFileReq * pMsg =
            reinterpret_cast<KEPlayRecordFileReq *>(msgData.data());
    std::string fileName = pMsg->fileData;

    if(pMsg->fileType == 1){
        LOG(INFO)<< "KeMessageProcessCamera::RecvPlayFile---start play"<<
                    pMsg->fileData;
        if(recordReader != NULL && recordReader->IsReading()){
            LOG(INFO)<<"KeMessageProcessCamera::RecvPlayFile---"<<
                       "already start record read";
            RespPlayFileReq(RESP_NAK);
            return;
        }
        if(recordReader == NULL){
            recordReader = new RecordReaderAvi();
            recordReader->SignalAudioData.connect(this,&KeMessageProcessCamera::OnAudioData);
            recordReader->SignalVideoData.connect(this,&KeMessageProcessCamera::OnVideoData);
            recordReader->SignalRecordEnd.connect(this,&KeMessageProcessCamera::OnRecordReadEnd);
            recordReader->SignalReportProgress.connect(this,&KeMessageProcessCamera::OnRecordProcess);
        }
        if(!recordReader->StartRead(fileName)){
            LOG_T_F(WARNING)<<"KeMessageProcessCamera::RecvPlayFile---"<<"start read failed";
            RespPlayFileReq(RESP_NAK);
            delete recordReader;
            recordReader = NULL;
            return;
        }
        this->videoInfo_ = recordReader->recordInfo;
        RespPlayFileReq(RESP_ACK);
    }else if(pMsg->fileType == 2){
        LOG_T_F(INFO)<<"play file control--position:"<< pMsg->playPos <<
                       " ,speed:"<<pMsg->playSpeed;

        if(pMsg->playSpeed != -1){
            recordReader->SetSpeed(pMsg->playSpeed);
            this->RespPlayFileReq(RESP_CTRL);
        }
        if(pMsg->playPos != -1){
            recordReader->SetPosition(pMsg->playPos);
            this->RespPlayFileReq(RESP_CTRL);
        }
    }else if(pMsg->fileType == 3){
        LOG_T_F(INFO)<<"stop play file";
        if(recordReader != NULL){
            recordReader->StopRead();
        }else{
            LOG_T_F(WARNING)<<"stop play file error file not start";

        }
//        recordReader->StopRead();
    }else{
        LOG_T_F(WARNING)<<"unknown file type";
    }

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

void KeMessageProcessCamera::RespPlayFileReq(int resp)
{
    talk_base::Buffer sendBuf;
    int msgLen = sizeof(KEPlayRecordFileResp);
    sendBuf.SetLength(msgLen);
    KEPlayRecordFileResp * msg = (KEPlayRecordFileResp *)sendBuf.data();
    msg->protocal = PROTOCOL_HEAD;
    msg->msgType = KEMSG_REQUEST_PLAY_FILE;
    msg->msgLength = msgLen;
    msg->videoID = 0;
    msg->playSpeed = recordReader->GetSpeed();
    msg->playPos = recordReader->GetPosition();
    msg->resp = resp;
    msg->frameRate = recordReader->recordInfo.frameRate;
    msg->frameResolution = recordReader->recordInfo.frameResolution;
//    talk_base::strcpyn(msg->fileName,80,fileName);
    SignalNeedSendData(this->peer_id(),sendBuf.data(),sendBuf.length());
}

void KeMessageProcessCamera::ConnectMedia(int video, int audio, int talk)
{
    KeTunnelCamera * camera = static_cast<KeTunnelCamera *>(container_);
    //get video info from container
    camera->GetCameraVideoInfo(video,&this->videoInfo_);
    LOG(INFO)<<"KeMessageProcessCamera::ConnectMedia--- video "<<video<<
               " listen-"<<audio<<" talk-"<<talk<<"; frameResolution="<<
               this->videoInfo_.frameResolution<<" ;framerate="<<videoInfo_.frameRate;

    if(video == 0){//stop
        camera->SignalVideoData1.disconnect(this);
        camera->SignalVideoData2.disconnect(this);
        camera->SignalVideoData3.disconnect(this);
        video_status = video;
    }
    else if(0 == video_status){
        this->RespAskMediaReq(this->videoInfo_);
        video_status = video;
        if(video == 1){
            camera->SignalVideoData1.connect(
                        this , &KeMessageProcessCamera::OnVideoData);
        }
        else if(video == 2){
            camera->SignalVideoData2.connect(
                        this , &KeMessageProcessCamera::OnVideoData);
        }
        else if(video == 3){
            camera->SignalVideoData3.connect(
                        this , &KeMessageProcessCamera::OnVideoData);
        }
    }
    if(audio == 0){
        camera->SignalAudioData.disconnect(this);
        audio_status = 0;
    }
    else if(0 == audio_status){
        audio_status = audio;
        camera->SignalAudioData.connect(
                    this,&KeMessageProcessCamera::OnAudioData);
    }
    if(talk == 0){
        this->SignalRecvTalkData.disconnect(camera);
        talk_status = 0;
    }else if(0 == talk_status){
        this->SignalRecvTalkData.connect(camera,&KeTunnelCamera::OnRecvTalkData);
        talk_status = talk;
    }
}

void KeMessageProcessCamera::OnRecordProcess(int percent)
{
    talk_base::Buffer sendBuf;
    int msgLen = sizeof(KEPlayRecordFileResp);
    sendBuf.SetLength(msgLen);
    KEPlayRecordFileResp * msg = (KEPlayRecordFileResp *)sendBuf.data();
    msg->protocal = PROTOCOL_HEAD;
    msg->msgType = KEMSG_REQUEST_PLAY_FILE;
    msg->msgLength = msgLen;
    msg->videoID = 0;
    msg->playSpeed = -1;
    msg->playPos = percent;
    msg->resp = RESP_CTRL;
    SignalNeedSendData(this->peer_id(),sendBuf.data(),sendBuf.length());
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
    } else {
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
    this->RespPlayFileReq(RESP_END);
    recordReader->StopRead();

//    delete recordReader;
//    recordReader = NULL;

//    int msgLen = sizeof(KEPlayRecordDataHead);
//    talk_base::Buffer sendBuf;
//    KEPlayRecordDataHead streamHead;
//    streamHead.protocal = PROTOCOL_HEAD;
//    streamHead.msgType = KEMSG_RecordPlayData;
//    streamHead.msgLength = msgLen;
//    streamHead.channelNo = 1;
//    streamHead.videoID = 0;
//    streamHead.resp = RESP_END;
//    sendBuf.AppendData(&streamHead,sizeof(KEPlayRecordDataHead));
//    SignalNeedSendData(this->peer_id(),sendBuf.data(),sendBuf.length());
}

}
