#include "kesdkdevice.h"

#include "talk/base/logging.h"
#include "talk/base/stringutils.h"
#include "talk/base/bind.h"
#include "talk/base/thread.h"
#include "talk/base/buffer.h"
#include "talk/base/stringencode.h"
#include "talk/base/base64.h"


#include "keapi/common_api.h"
#include "keapi/common_define.h"
#include "keapi/media_api.h"

#include "libjingle_app/defaults.h"
#include "libjingle_app/jsonconfig.h"
#include "libjingle_app/KeMessage.h"


//enum FramTypes{
//    FRAM_D1 = 0,
//    FRAM_QCIF = 1,
//    FRAM_CIF = 2,
//    FRAM_HD1 = 3,
//    FRAM_QVGA = 4,
//    FRAM_VGA = 7,
//    FRAM_720P = 10
//};

enum StreamLevel{
    kLevelNoneStream = 0,
    kLevelMainStream = 1,
    kLevelSubStream = 2,
    kLevelExtStream = 3,
    kLevelAudioStream
};

int ResoToFramType(int resoValue){
    switch(resoValue){
    case RESO_CIF:return kaerp2p::kFrameCIF;
    case RESO_D1:return kaerp2p::kFrameD1;
    case RESO_720P:return kaerp2p::kFrame720P;
    default:
        LOG_F(WARNING)<<"false";
        return 0;
    }
}


KeSdkDevice::KeSdkDevice():video1_handle_(-1),video2_handle_(-1),video3_handle_(-1),
    audio_handle_(-1)
{

    CONFIG_Initialize();

//    struct NETPARAM	net;
//    CONFIG_Get(CONFIG_TYPE_NET,(void *)&net);
//    LOG_F(INFO)<< "local ip is "<<(int)net.localIP[0]<<"."<<(int)net.localIP[1]<<"."<<
//                 (int)net.localIP[2]<<"."<<(int)net.localIP[3];
//    std::string ret((const char *)net.macAddress);
//    LOG_F(INFO)<<"get mac id is "<<ret;


//    NET_Initialize();
//    struct MEDIAPARAM media;
//    CONFIG_Get(CONFIG_TYPE_MEDIA,(void *)&media);
//    printf("resolution %d frame_rate %d rate_ctrl_mode %d bitrate %d piclevel %d\n",
//            media.main[0].resolution,media.main[0].frame_rate,
//            media.main[0].rate_ctrl_mode,media.main[0].bitrate,
//            media.main[0].piclevel);

//    media.main[0].frame_rate = 25;
//    media.main[0].resolution = RESO_720P;
//    media.main[0].bitrate = 512;
//    media.main[0].piclevel = 0;
//    media.minor[0].frame_rate = 25;
//    media.minor[0].resolution = RESO_D1;
//    media.minor[0].bitrate = 256;
//    media.minor[0].piclevel = 0;

//    CONFIG_Set(CONFIG_TYPE_MEDIA,(void *)&media);

}

KeSdkDevice::~KeSdkDevice()
{
    MEDIA_Cleanup();
    NET_Cleanup();
    CONFIG_Cleanup();


}

bool KeSdkDevice::Init(kaerp2p::PeerTerminalInterface *t)
{

    InitVideoInfo();

    RegisterCallBack::Instance()->SignalVideoFrame.connect(
                this,&KeSdkDevice::SendVideoFrame);
    RegisterCallBack::Instance()->SignalAudioFrame.connect(
                this,&KeSdkDevice::SendAudioFrame);

    MEDIA_Initialize();

//    video1_handle_ = FIFO_Stream_Open(FIFO_STREAM_H264,0,0);
//    video2_handle_ = FIFO_Stream_Open(FIFO_STREAM_H264,0,1);
//    video3_handle_ = FIFO_Stream_Open(FIFO_STREAM_H264,0,2);
//    audio_handle_ = FIFO_Stream_Open(FIFO_STREAM_AUDIO,0,0);

    return KeTunnelCamera::Init(t);
}

void KeSdkDevice::GetCameraVideoInfo(int level, kaerp2p::VideoInfo *info)
{
    if(level == 1){
        *info = this->video1_info_;
    }
    else if(level == 2){
        *info = this->video2_info_;
    }
    else if(level == 3){
        *info = this->video3_info_;
    }
}

void KeSdkDevice::OnTunnelOpened(kaerp2p::PeerTerminalInterface *t, const std::string &peer_id)
{
    ASSERT(terminal_ == t);
    LOG(INFO)<<"KeSdkDevice::OnTunnelOpened---"<<peer_id;
    KeSdkProcess *process = new KeSdkProcess(peer_id,this);
    this->AddMsgProcess(process);
}

void KeSdkDevice::OnRecvTalkData(const std::string &peer_id, const char *data, int len)
{
//    LOG_T_F(INFO)<<" talk data "<< len;
    KEFrameHead * head = (KEFrameHead *)data;
    int dataPos =  sizeof(KEFrameHead);
    if(head->frameLen == len - dataPos){
        audioStreamDown(const_cast<char *>(data+dataPos),head->frameLen);
    }else{
        LOG_F(WARNING)<<"tal from "<<peer_id<<" frame format error";
    }

}

void KeSdkDevice::OnCommandJsonMsg(const std::string &peerId, Json::Value &jmessage)
{
    std::string command;
    bool ret = GetStringFromJsonObject(jmessage,kaerp2p::kKaerMsgCommandName, &command);
    if(!ret){
        LOG(WARNING)<<"get command error-"<<command<<" from"<<peerId ;
        return;
    }
    Json::Value jresult;

    if(command.compare("ptz") == 0){
        std::string ptz_control;
        GetStringFromJsonObject(jmessage,"control",&ptz_control);
        int param;
        GetIntFromJsonObject(jmessage,"param",&param);
        //std::string ptz_key = "ptz_";
        //ptz_key += ptz_control;
        //this->SetPtz(ptz_key,param);
        this->SetPtz(ptz_control,param);
    }
    else if(command.compare("query_record") == 0){
        Json::Value jcondition;
        if(!GetValueFromJsonObject(jmessage, "condition", &jcondition))
        {
            LOG(WARNING)<<"get query_record value error from" << peerId ;
            return;
        }
        std::string condition = JsonValueToString(jcondition);
        LOG_F(INFO)<<condition;
        //OnRecvRecordQuery(peerId,condition);
    }
    else if(command.compare("wifi_info") == 0){
        //this->RecvGetWifiInfo(peerId);

        Json::Value jwifi = GetWifiJsonArray();

        jresult["type"] = "tunnel";
        jresult["command"] = "wifi_info";
        jresult["wifis"].append(jwifi);

    }else if(command.compare("set_wifi") == 0){
        Json::Value jwifiParam;
        if(!GetValueFromJsonObject(jmessage, "param", &jwifiParam)){
            LOG(WARNING)<<"get set_wifi value error from"<<peerId;
            return;
        }
        std::string jstrParam = JsonValueToString(jwifiParam);

        bool ret = this->SetWifiInfo(jstrParam);

        jresult["type"] = "tunnel";
        jresult["command"] = "set_wifi";
        jresult["result"] = ret;
    }
    else{
        kaerp2p::KeTunnelCamera::OnCommandJsonMsg(peerId,jmessage);
    }

    if(!jresult.isNull()){
        Json::StyledWriter writer;
        std::string msg = writer.write(jresult);
        LOG(LS_VERBOSE)<<"send msg is "<< msg;
        this->terminal_->SendByRouter(peerId,msg);
    }
}

void KeSdkDevice::OnMessage(talk_base::Message *msg)
{
    switch (msg->message_id) {
    case MSG_CheckCloseStream:

        break;
    default:
        break;
    }
}

void KeSdkDevice::SendVideoFrame(const char *data, int len, int level)
{
    talk_base::CritScope cs(&crit_);
    for(int i =0 ;i< processes_.size();i++){
        kaerp2p::KeMessageProcessCamera * camProcess =
                static_cast<kaerp2p::KeMessageProcessCamera *>(processes_[i]);
        if(camProcess->video_status == level){
            camProcess->OnVideoData(data,len);
        }
    }
}

void KeSdkDevice::SendAudioFrame(const char *data, int len)
{
    talk_base::CritScope cs(&crit_);
    for(int i =0 ;i< processes_.size();i++){
        kaerp2p::KeMessageProcessCamera * camProcess =
                static_cast<kaerp2p::KeMessageProcessCamera *>(processes_[i]);
        if(camProcess->audio_status == 1){
            camProcess->OnAudioData(data,len);
        }
    }

}
//level 1~4 , 1- video1 ,2-video2,3-video3,4-audio stream
bool KeSdkDevice::MediaStreamOpen(int level)
{
    switch (level) {
    case kLevelMainStream:
        if(video1_handle_ == -1){
            video1_handle_ = FIFO_Stream_Open(FIFO_STREAM_H264,0,0);
            LOG_F(INFO)<<"open video1 stream " << video1_handle_;
        }
        return true;
    case kLevelSubStream:
        if( video2_handle_ == -1){
            video2_handle_ = FIFO_Stream_Open(FIFO_STREAM_H264,0,1);
            LOG_F(INFO)<<"open video2 stream"<<video2_handle_;
        }
        return true;
    case kLevelExtStream:
        if( video3_handle_ == -1){
            video3_handle_ = FIFO_Stream_Open(FIFO_STREAM_H264,0,2);
            LOG_F(INFO)<<"open video3 stream"<<video3_handle_;
        }
        return true;
    case kLevelAudioStream:
        if( audio_handle_ == -1 ){
            audio_handle_ = FIFO_Stream_Open(FIFO_STREAM_AUDIO,0,0);
            LOG_F(INFO)<<"open audio stream"<<audio_handle_;
        }
        return true;
    default:
        LOG_F(WARNING)<<"error level "<<level;
        return false;
    }
}

void KeSdkDevice::MediaGetIDR(int level)
{
    if(level != kLevelMainStream && level != kLevelSubStream &&
            level != kLevelExtStream){
        LOG_F(WARNING)<<"level param error "<<level;
        return;
    }
    //subchnl 0-主通道 1-子通道 2-扩展通道
    MEDIA_Get_IDR(0,level-1);
}

std::string KeSdkDevice::GetMacAddress()
{
    struct NETPARAM	net;
    CONFIG_Get(CONFIG_TYPE_NET,(void *)&net);
    LOG_F(INFO) << "local ip is " << (int)net.localIP[0] << "."<<(int)net.localIP[1]<<"."<<
                 (int)net.localIP[2]<<"."<<(int)net.localIP[3];
    std::string ret((const char *)net.macAddress);
    LOG_F(INFO)<<"get mac id is "<<ret;
    return ret;
}

void KeSdkDevice::SetNtp(const std::string &ntpParam)
{
//    CLOCK_Set_NTP(,123,);
}


void KeSdkDevice::InitVideoInfo()
{
    struct MEDIAPARAM media;
    CONFIG_Get(CONFIG_TYPE_MEDIA,(void *)&media);
    printf("resolution %d frame_rate %d rate_ctrl_mode %d bitrate %d piclevel %d\n",
           media.main[0].resolution,media.main[0].frame_rate,
            media.main[0].rate_ctrl_mode,media.main[0].bitrate,media.main[0].piclevel);
    video1_info_.frameRate = media.main[0].frame_rate;
    video1_info_.frameResolution = ResoToFramType(media.main[0].resolution);
    video1_info_.frameInterval = 1000/video1_info_.frameRate;
    video2_info_.frameRate = media.minor[0].frame_rate;
    video2_info_.frameResolution = ResoToFramType(media.minor[0].resolution);
    video2_info_.frameInterval = 1000/video2_info_.frameRate;
    video3_info_.frameRate = 25;
    video3_info_.frameResolution = kaerp2p::kFrameCIF;
    video3_info_.frameInterval = 1000/video3_info_.frameRate;
}

bool KeSdkDevice::SetPtz(std::string control, int param)
{
    const int kDefaultSpeed  = 63;
    int cmd = 0;
    int speed = kDefaultSpeed;
    if(control.compare("stop") == 0){
        cmd = 0;
    }else if(control.compare("move_left")){
        cmd = 3;
        speed = param;
    }else if(control.compare("move_right")){
        cmd = 4;
        speed = param;
    }else if(control.compare("move_up")){
        cmd = 1;
        speed = param;
    }else if(control.compare("move_down")){
        cmd = 2;
        speed = param;
    }else{
        LOG_F(WARNING)<<" wrong ptz control";
        return false;
    }
    Control_MOTOR(0,cmd,kDefaultSpeed,param);
    return true;
}

Json::Value KeSdkDevice::GetWifiJsonArray()
{
    const int kMaxWifiList = 64;
    st_wifi_list_t wifiList[kMaxWifiList];
    int count = NET_Get_WifiList(kMaxWifiList,wifiList);
    Json::Value jmessage(Json::arrayValue);

    for(int i=0;i < count;++i){
        Json::Value jwifi;
        jwifi["ssid"] = wifiList[i].ssid;
        jwifi["signalStrength"] = wifiList[i].signalStrength;
        jwifi["encryptFormat"] = wifiList[i].encryptFormat;
        jwifi["encryptMode"] = wifiList[i].encryptMode;//加密方式
        jwifi["enable"] = wifiList[i].enable;
        jwifi["wepPosition"] = wifiList[i].wepPosition;
        jmessage.append(jwifi);
    }
    return jmessage;
}

bool KeSdkDevice::SetWifiInfo(Json::Value jparam)
{
    st_wifi_list_t wifiParam;
    std::string ssid;
    GetStringFromJsonObject(jparam,"ssid",&ssid);
    talk_base::asccpyn(wifiParam.ssid,32,ssid.c_str());
    std::string key;
    GetStringFromJsonObject(jparam,"key",&key);
    talk_base::asccpyn(wifiParam.key,32,key.c_str());
    GetIntFromJsonObject(jparam,"enable",&wifiParam.enable);
    int encryptMode;
    GetIntFromJsonObject(jparam,"encryptMode",&encryptMode);
    wifiParam.encryptMode =(e_encrypt_mode) encryptMode;
    int encryptFormat;
    GetIntFromJsonObject(jparam,"encryptFormat",&encryptFormat);
    wifiParam.encryptFormat =(e_encrypt_format) encryptFormat;
    GetIntFromJsonObject(jparam,"wepPosition",&wifiParam.wepPosition);

    int ret = NET_Set_Wifi(&wifiParam);
    LOG_F(INFO)<<"NET_Set_Wifi "<<ret;
    if(ret == 0){
        return false;
    }else{
        return true;
    }
}


KeSdkDevice::RegisterCallBack::RegisterCallBack()
{
    FIFO_Register_Callback(FIFO_H264_MAIN,&RegisterCallBack::MainStreamCallBack);
    FIFO_Register_Callback(FIFO_H264_SUB,&RegisterCallBack::SubStreamCallBack);
    FIFO_Register_Callback(FIFO_H264_EXT,&RegisterCallBack::ExtStreamCallBack);
    FIFO_Register_Callback(FIFO_H264_AUDIO,&RegisterCallBack::AudioStreamCallBack);
}

KeSdkDevice::RegisterCallBack *KeSdkDevice::RegisterCallBack::Instance(){
    static RegisterCallBack instance;
    return &instance;
}

int KeSdkDevice::RegisterCallBack::MainStreamCallBack(char *pFrameData, int iFrameLen)
{
    RegisterCallBack::Instance()->SignalVideoFrame(pFrameData,iFrameLen,1);
}

int KeSdkDevice::RegisterCallBack::SubStreamCallBack(char *pFrameData, int iFrameLen)
{
    RegisterCallBack::Instance()->SignalVideoFrame(pFrameData,iFrameLen,2);
}

int KeSdkDevice::RegisterCallBack::ExtStreamCallBack(char *pFrameData, int iFrameLen)
{
    RegisterCallBack::Instance()->SignalVideoFrame(pFrameData,iFrameLen,3);
}

int KeSdkDevice::RegisterCallBack::AudioStreamCallBack(char *pFrameData, int iFrameLen)
{
    RegisterCallBack::Instance()->SignalAudioFrame(pFrameData,iFrameLen);
}


KeSdkProcess::KeSdkProcess(std::string peerId, KeSdkDevice *container):
    kaerp2p::KeMessageProcessCamera(peerId,container)
{

}

void KeSdkProcess::ConnectMedia(int video, int audio, int talk)
{
    KeSdkDevice * camera = static_cast<KeSdkDevice *>(container_);
    //get video info from container
    camera->GetCameraVideoInfo(video,&this->videoInfo_);
    LOG(INFO)<<"KeMessageProcessCamera::ConnectMedia--- video "<<video<<
               " listen-"<<audio<<" talk-"<<talk<<"; frameResolution="<<
               this->videoInfo_.frameResolution<<" ;framerate="<<videoInfo_.frameRate;

    if(video == 0){//stop
        video_status = video;
    }
    else if(0 == video_status){
        this->RespAskMediaReq(this->videoInfo_);
        video_status = video;
        //try to open stream
        camera->MediaStreamOpen(video);
        camera->MediaGetIDR(video);
    }else{ //video > 0
        camera->MediaGetIDR(video);
    }

    if(audio == 0){
        audio_status = 0;
    }
    else if(0 == audio_status){
        audio_status = audio;
        camera->MediaStreamOpen(4);
    }

    if(talk == 0){
        this->SignalRecvTalkData.disconnect(camera);
        talk_status = 0;
    }
    else if(0 == talk_status){
        this->SignalRecvTalkData.connect(camera,&KeSdkDevice::OnRecvTalkData);
        talk_status = talk;
    }

}
