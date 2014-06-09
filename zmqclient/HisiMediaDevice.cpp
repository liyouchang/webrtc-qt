#include "HisiMediaDevice.h"

#include <sstream>

#include "talk/base/bind.h"
#include "talk/base/thread.h"
#include "talk/base/buffer.h"
#include "talk/base/logging.h"
#include "talk/base/stringutils.h"
#include "talk/base/stringencode.h"
#include "talk/base/base64.h"

#include "keapi/keapi.h"

#include "libjingle_app/KeMessage.h"
#include "libjingle_app/KeMsgProcess.h"
#include "libjingle_app/jsonconfig.h"
#include "libjingle_app/defaults.h"

#define VIDEO1_DATA				"video1_data"
#define VIDEO1_CODEC 			"video1_codec"      	// 	0:H264, 1:MJPG
#define VIDEO1_BITRATECTRL 		"video1_control"		// 	0:CBR,  1:VBR
#define VIDEO1_BITRATE 			"video1_bitrate"		//
#define VIDEO1_QUALITY 			"video1_quality"		// 	5:普通,7:较好,9:最好
#define VIDEO1_FRAMERATE 		"video1_framerate"		//	1,5,10,15,20,25,30
#define VIDEO1_IFRAMEINTERVAL 	"video1_int"			// 	1,4,20,100,200
#define VIDEO1_DEINTERLACE 		"video1_deint"			// 	0:close,1:open反交错
#define VIDEO1_RESOLUTION 		"video1_resolution"		// 	176x144, 352x288, 320x240 640x480 1280x720


#define VIDEO2_DATA				"video2_data"
#define VIDEO2_CODEC			"video2_codec"			// 	0:H264, 1:MJPG 0:
#define VIDEO2_BITRATECTRL		"video2_control"		// 	0:CBR,  1:VBR
#define VIDEO2_BITRATE 		 	"video2_bitrate"		//
#define VIDEO2_QUALITY 		 	"video2_quality"     	// 	5:普通,7:较好,9:最好
#define VIDEO2_FRAMERATE		"video2_framerate"		//	1,5,10,15,20,25,30,
#define VIDEO2_IFRAMEINTERVAL	"video2_int"			//	1,4,20,100,200
#define VIDEO2_RESOLUTION		"video2_resolution"		//	176x144, 352x288, 320x240 640x480


#define AUDIO1_DATA				"audio_data"

#define PTZ_MOVE_LEFT "ptz_move_left"
#define PTZ_MOVE_RIGTE "ptz_move_right"
#define PTZ_MOVE_UP "ptz_move_up"
#define PTZ_MOVE_DOWN "ptz_move_down"
#define PTZ_STOP "ptz_stop"

#define HARDWARE_ID "hardware_id"

const int kVideoSampleRate = 40;//40 ms per frame
const int kAudioSampleRate = 20;//20 ms

struct MediaControlData : public talk_base::MessageData {
    int video1;//1 to request start video1 , 0 to request stop
    int video2;//1 to request start video2, 0 to request stop
    int audio;//1 to request start audio, 0 to request stop
    MediaControlData(int v1,int v2 ,int a) : video1(v1),video2(v2), audio(a) { }
};


HisiMediaDevice::HisiMediaDevice():
    media_thread_(0),video1_handle_(0),video2_handle_(0),audio_handle_(0)
{
    int ret = Raycomm_InitParam();
    LOG(INFO)<<"Raycomm_InitParam : "<<ret;
    ret = Raycomm_MediaDataInit();
    LOG(INFO)<<"Raycomm_MediaDataInit : "<<ret;
    InitDeviceVideoInfo();
}

HisiMediaDevice::~HisiMediaDevice()
{
    if( media_thread_){
        delete media_thread_;
    }
    Raycomm_MediaDataUnInit();
    Raycomm_UnInitParam();
}

bool HisiMediaDevice::Init(kaerp2p::PeerConnectionClientInterface *client)
{
    media_thread_ = new talk_base::Thread();
    media_thread_->Start();
    //start get media
    media_thread_->Post(this, MSG_MEDIA_CONTROL, new MediaControlData(1,1,1));
    AlarmNotify::Instance()->StartNotify();
    return KeTunnelCamera::Init(client);
}

bool HisiMediaDevice::InitDeviceVideoInfo()
{
    video1_info_.frameRate = this->GetVideoFrameRate(1);
    video1_info_.frameResolution = this->GetVideoFrameType(1);
    video2_info_.frameRate = this->GetVideoFrameRate(2);
    video2_info_.frameResolution = this->GetVideoFrameType(2);
    return true;
}



void HisiMediaDevice::SendVideoFrame(const char *data, int len, int level)
{
    if(level == 1){
        SignalVideoData1(data,len);
    }else if(level == 2){
        SignalVideoData2(data,len);
    }
}

void HisiMediaDevice::SendAudioFrame(const char *data, int len)
{
    SignalAudioData(data,len);
}

void HisiMediaDevice::SetVideoClarity(int clarity)
{
    if(clarity == 1){
        SetVideoResolution("352x288");
    }else if(clarity == 2){
        SetVideoResolution("704x576");
    }else if(clarity == 3){
        SetVideoResolution("1280x720");
    }else{
        LOG(WARNING)<<"clarity value error";
        return;
    }
    JsonConfig::Instance()->Set("clarity",clarity);
    JsonConfig::Instance()->ToFile(kConfigFileName);
}

int HisiMediaDevice::GetVideoClarity()
{
    Json::Value jclarity = JsonConfig::Instance()->Get("clarity",2);
    return jclarity.asInt();
}

void HisiMediaDevice::SetPtz(std::string ptz_key, int param)
{
    LOG(INFO)<<"HisiMediaDevice::SetPtz---key:" <<ptz_key<<" param:"<<param ;
    Raycomm_SetPtz(ptz_key.c_str(),param,0);
}

void HisiMediaDevice::OnRecvTalkData(const std::string &peer_id, const char *data, int len)
{
    KEFrameHead * head = (KEFrameHead *)data;
    int dataPos =  sizeof(KEFrameHead);
    if(head->frameLen == len - dataPos){
        Raycomm_TalkPlay(0,const_cast<char *>(data+dataPos),head->frameLen,0,0);
    }else{
        LOG(WARNING)<<"HisiMediaDevice::OnRecvTalkData--- from "<<peer_id<<
                    " frame format error";
    }
}

void HisiMediaDevice::RecvGetWifiInfo(std::string peer_id)
{
    LOG(INFO)<<"HisiMediaDevice::RecvGetWifiInfo---"<<peer_id;
    t_WIFI_INFO * wifiList = new t_WIFI_INFO[64];
    int count;
    int ret = Raycomm_GetWifiList(&count,wifiList);
    LOG(INFO)<<"Raycomm_GetWifiList ret-"<<ret<<" count-"<<count;
    Json::Value jmessage;
    jmessage["type"] = "tunnel";
    jmessage["command"] = "wifi_info";
    for(int i=0;i < count;++i){
        Json::Value jwifi;
        jwifi["ssid"] = wifiList[i].sSsID;
        jwifi["quality"] = wifiList[i].u32Quality;
        jwifi["auth"] = wifiList[i].u32Auth;
        jwifi["enc"] = wifiList[i].u32Enc;
        jwifi["mode"] = wifiList[i].u32Mode;
        jmessage["wifis"].append(jwifi);
    }
    Json::StyledWriter writer;
    std::string msg = writer.write(jmessage);
    LOG(LS_VERBOSE)<<"send msg is "<< msg;
    this->terminal_->SendByRouter(peer_id,msg);
}

void HisiMediaDevice::SetWifiInfo(std::string peerId, std::string param)
{
    Json::Reader reader;
    Json::Value jparam;
    if (!reader.parse(param,jparam)) {
        LOG(WARNING) << "Received unknown message. " << param;
        return;
    }
    t_WIFI_PARAM wifiParam;
    std::string ssid;
    GetStringFromJsonObject(jparam,"ssid",&ssid);
    talk_base::asccpyn(wifiParam.sSsID,32,ssid.c_str());
    std::string key;
    GetStringFromJsonObject(jparam,"key",&key);
    talk_base::asccpyn(wifiParam.sKey,32,key.c_str());
    GetIntFromJsonObject(jparam,"enable",&wifiParam.u32Enable);
    GetIntFromJsonObject(jparam,"auth",&wifiParam.u32Auth);
    GetIntFromJsonObject(jparam,"enc",&wifiParam.u32Enc);
    GetIntFromJsonObject(jparam,"mode",&wifiParam.u32Mode);
    LOG(INFO)<<"Raycomm_SetWifi --- param key "<<wifiParam.sKey;
    int ret = Raycomm_SetWifi(&wifiParam);
    LOG(INFO)<<"Raycomm_SetWifi --- ret:"<<ret<<" param:"<<param;
    Json::Value jmessage;
    jmessage["type"] = "tunnel";
    jmessage["command"] = "set_wifi";
    jmessage["result"] = ret;
    Json::StyledWriter writer;
    std::string msg = writer.write(jmessage);
    this->terminal_->SendByRouter(peerId,msg);
}

void HisiMediaDevice::OnRecvRecordQuery(std::string peer_id, std::string condition)
{
    LOG(INFO)<<"HisiMediaDevice::OnRecvRecordQuery ---"<<
               peer_id<<" query:"<<condition;

    int totalNum = 0;
    t_VidRecFile_QueryInfo videoRecordList;
    Json::Reader reader;
    Json::Value jcondition;
    if (!reader.parse(condition, jcondition)) {
        LOG(WARNING) << "Received unknown message. " << condition;
        totalNum = -1;
    }else{
        std::string startTime,endTime;
        GetStringFromJsonObject(jcondition,"startTime",&startTime);
        GetStringFromJsonObject(jcondition,"endTime",&endTime);
        int offset,toQuery;
        GetIntFromJsonObject(jcondition,"offset",&offset);
        GetIntFromJsonObject(jcondition,"toQuery",&toQuery);
        totalNum = Raycomm_QueryNVR(
                    (char *)startTime.c_str(),(char *)endTime.c_str(),
                    &videoRecordList,offset,toQuery);
    }

    Json::StyledWriter writer;
    Json::Value jmessage;
    jmessage["type"] = "tunnel";
    jmessage["command"] = "query_record";
    jmessage["condition"] = jcondition;
    jmessage["totalNum"] = totalNum;
    for(int i = 0;i < videoRecordList.file_num;i++){
        Json::Value jrecord;
        jrecord["fileName"] = videoRecordList.rec_file[i].path;
        jrecord["fileDate"] = videoRecordList.rec_file[i].date;
        jrecord["fileSize"] = videoRecordList.rec_file[i].size;
        jmessage["recordList"].append(jrecord);
    }
    std::string msg = writer.write(jmessage);
    this->terminal_->SendByRouter(peer_id,msg);

}


void HisiMediaDevice::OnMessage(talk_base::Message *msg)
{
    switch (msg->message_id) {
    case MSG_MEDIA_CONTROL:{
        MediaControlData* mcd = static_cast<MediaControlData*>(msg->pdata);
        if(mcd->video1 == 1 && video1_handle_ == 0){
            video1_handle_ =  Raycomm_ConnectMedia(VIDEO1_DATA,0);
            LOG(INFO)<<"HisiMediaDevice start video1 " <<video1_handle_;
            media_thread_->Post(this,HisiMediaDevice::MSG_SEND_VIDEO);
        }else if(mcd->video1 == 0 && video1_handle_ != 0){
            Raycomm_DisConnectMedia(video1_handle_);
            LOG(INFO)<<"HisiMediaDevice stop video";
            video1_handle_ = 0;
        }

        if(mcd->video2 == 1 && video2_handle_ == 0){
            video2_handle_ =  Raycomm_ConnectMedia(VIDEO2_DATA,0);
            LOG(INFO)<<"HisiMediaDevice start video2 " <<video2_handle_;
            media_thread_->Post(this,HisiMediaDevice::MSG_SEND_VIDEO_SUB);
        }else if(mcd->video2 == 0 && video2_handle_ != 0){
            Raycomm_DisConnectMedia(video2_handle_);
            LOG(INFO)<<"HisiMediaDevice stop video";
            video2_handle_ = 0;
        }

        if(mcd->audio == 1 && audio_handle_ == 0 ){
            audio_handle_ =  Raycomm_ConnectMedia(AUDIO1_DATA,0);
            LOG(INFO)<<"HisiMediaDevice start audio" <<audio_handle_;
            media_thread_->Post(this,HisiMediaDevice::MSG_SEND_AUDIO);
        }else if(mcd->audio == 0 && audio_handle_ != 0){
            Raycomm_DisConnectMedia(audio_handle_);
            LOG(INFO)<<"HisiMediaDevice stop audio" ;
            audio_handle_ = 0;
        }

        delete msg->pdata;
        break;
    }
    case HisiMediaDevice::MSG_SEND_VIDEO:{
        if(video1_handle_ == 0) break;
        unsigned int timespan;
        int media_len = Raycomm_GetMediaData(video1_handle_,media_buffer_,
                                             MEDIA_BUFFER_LENGTH,&timespan);
        if(media_len > 0){
            if(media_len == MEDIA_BUFFER_LENGTH){
                LOG(LS_ERROR)<<"Raycomm_GetMediaData--- video1 "<<
                               "no enough buffer for this large frame";
            }
            this->SendVideoFrame(media_buffer_,media_len,1);
            //get next frame
            media_thread_->Post(this,HisiMediaDevice::MSG_SEND_VIDEO);
        }else{
            //wait and get
            media_thread_->PostDelayed(kVideoSampleRate,this,
                                       HisiMediaDevice::MSG_SEND_VIDEO);
        }
        break;
    }
    case HisiMediaDevice::MSG_SEND_VIDEO_SUB:{
        if(video2_handle_ == 0) break;
        unsigned int timespan;
        int media_len = Raycomm_GetMediaData(video2_handle_,media_buffer_,
                                             MEDIA_BUFFER_LENGTH,&timespan);
        if(media_len > 0){
            if(media_len == MEDIA_BUFFER_LENGTH){
                LOG(LS_ERROR)<<"Raycomm_GetMediaData--- video2 "<<
                               "no enough buffer for this large frame";
            }
            this->SendVideoFrame(media_buffer_,media_len,2);
            //get next frame
            media_thread_->Post(this,HisiMediaDevice::MSG_SEND_VIDEO_SUB);
        }else{
            //wait and get
            media_thread_->PostDelayed(kVideoSampleRate,this,
                                       HisiMediaDevice::MSG_SEND_VIDEO_SUB);
        }
        break;
    }
    case HisiMediaDevice::MSG_SEND_AUDIO:{
        if(audio_handle_ == 0) break;
        unsigned int timespan;
        int media_len = Raycomm_GetMediaData(audio_handle_,media_buffer_,
                                             MEDIA_BUFFER_LENGTH,&timespan);
        if(media_len > 0){
            this->SendAudioFrame(media_buffer_,media_len);
            media_thread_->Post(this,HisiMediaDevice::MSG_SEND_AUDIO);
        }else{
            media_thread_->PostDelayed(kAudioSampleRate,this,
                                       HisiMediaDevice::MSG_SEND_AUDIO);
        }
        break;
    }
    default:
        break;
    }
}

void HisiMediaDevice::SetVideoResolution(std::string r)
{
    std::string command = VIDEO1_RESOLUTION;
    command+="=";
    command+=r;
    Raycomm_SetParam((char*)command.c_str(),0);
}

int HisiMediaDevice::GetVideoFrameType(int level)
{
    char buf[1024];
    memset(buf,0,1024);
    char * key;
    if(level == 1){
        key = VIDEO1_RESOLUTION;
    }else if(level == 2){
        key = VIDEO2_RESOLUTION;
    }
    Raycomm_GetParam(key,buf,0);
    int frameType;
    std::string resolution(buf);
    if(resolution.compare("352x288") == 0){//CIF
        frameType = 2;
    }else if(resolution.compare("704x576") == 0){//D1
        frameType = 0;
    }else if(resolution.compare("1280x720") == 0){//720P
        frameType = 10;
    }else if(resolution.compare("176x144") == 0){//QCIF
        frameType = 1;
    }else if(resolution.compare("704x288") == 0){//HD1
        frameType = 3;
    }else if(resolution.compare("320x240") == 0){//QVGA
        frameType = 4;
    }else if(resolution.compare("640x480") == 0){//VGA
        frameType = 7;
    }else{
        LOG(WARNING)<<"GetVideoFrameType get not defined frame resolution "<<
                      resolution;
        frameType = 2;
    }
    //LOG(INFO)<<"HisiMediaDevice::GetVideoFrameType---"<<frameType;
    return frameType;
}

int HisiMediaDevice::GetVideoFrameRate(int level)
{
    char buf[1024];
    memset(buf,0,1024);
    char * key;
    if(level == 1){
        key = VIDEO1_FRAMERATE;
    }else if(level == 2){
        key = VIDEO2_FRAMERATE;
    }
    Raycomm_GetParam(key,buf,0);
    int frameRate = atoi(buf);
    //LOG(INFO)<<"HisiMediaDevice::GetVideoFrameRate---"<<frameRate;
    return frameRate;
}

std::string HisiMediaDevice::GetHardwareId()
{
    char buf[1024];
    memset(buf,0,1024);
    Raycomm_GetParam(HARDWARE_ID,buf,0);
    std::string result = buf;
    return result;
}

void HisiMediaDevice::GetCameraVideoInfo(int level, kaerp2p::VideoInfo *info)
{
    if(level == 1){
        *info = this->video1_info_;
    }else if(level == 2){
        *info = this->video2_info_;
    }
}



void AlarmNotify::StartNotify()
{
    //TODO: to start alarm notify
    int ret = Raycomm_Register_Callback(&AlarmNotify::NotifyCallBack);
    LOG(INFO)<<"AlarmNotify::StartNotify---Raycomm_Register_Callback:"<<ret;
}


//通道号   chn = 1.2.3.4
//移动侦测 rea = 1
//遮挡报警 rea = 4
//开关量   rea = 5
//门磁    rea = 2，io = 20
//人体红外 rea = 2，io = 21
//烟感报警 rea = 2，io = 22
int AlarmNotify::NotifyCallBack(int chn, int rea, int io,
                                int snapcount, int snapsize, char *snapbuf)
{
    //TODO: to signal alarm message
    LOG(INFO)<<"AlarmNotify::NotifyCallBack---chn:"<<chn<<" rea:"<<rea
            <<" io:"<<io << " snapcout:"<<snapcount<<" snapsize:" << snapsize;
    std::ostringstream infostream;
    infostream<<"发生";
    if(rea == 1){
        infostream<<" 移动侦测报警";
    }else if(rea == 4){
        infostream<<" 遮挡报警";
    }else if(rea==5){
        infostream<<" 开关量报警";
    }else if(rea==2){
        if(io == 20){
            infostream<<" 门磁报警";
        }else if(io == 21){
            infostream<<" 人体红外报警";
        }else if(io == 22){
            infostream<<" 烟感报警";
        }else{
            infostream<<" unknown";
        }
    } else {
        infostream<<" unknown";
    }
    std::string picBase64Data;
    if (snapsize > 0){
        talk_base::Base64::EncodeFromArray(snapbuf,snapsize,&picBase64Data);
    }
    LOG(INFO)<<"picture base64 data:"<<picBase64Data;
    int alarmType = rea;
    AlarmNotify::Instance()->SignalTerminalAlarm(alarmType,infostream.str(),
                                                 picBase64Data);
    return 0;
}


KaerCameraProcess::KaerCameraProcess(std::string peerId,
                                     kaerp2p::KeTunnelCamera *container):
    kaerp2p::KeMessageProcessCamera(peerId,container)
{

}
