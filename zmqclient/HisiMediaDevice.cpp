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
#include "libjingle_app/defaults.h"
#include "libjingle_app/KeMessage.h"
#include "libjingle_app/KeMsgProcess.h"
#include "libjingle_app/jsonconfig.h"

#define VIDEO1_DATA			"video1_data"
#define VIDEO1_CODEC 			"video1_codec"          //0:H264, 1:MJPG
#define VIDEO1_BITRATECTRL 		"video1_control"        //0:CBR,  1:VBR
#define VIDEO1_BITRATE 			"video1_bitrate"	//
#define VIDEO1_QUALITY 			"video1_quality"	//5:普通,7:较好,9:最好
#define VIDEO1_FRAMERATE 		"video1_framerate"	//,5,10,15,20,25,30
#define VIDEO1_IFRAMEINTERVAL           "video1_int"		//1,4,20,100,200
#define VIDEO1_DEINTERLACE 		"video1_deint"		//0:close,1:open反交错
#define VIDEO1_RESOLUTION 		"video1_resolution"	//176x144, 352x288, 320x240 640x480 1280x720

#define VIDEO2_ENABLE 			"video2_enable"		//0:disable 1:enable
#define VIDEO2_DATA                     "video2_data"
#define VIDEO2_CODEC			"video2_codec"		//0:H264, 1:MJPG 0:
#define VIDEO2_BITRATECTRL		"video2_control"	//0:CBR,  1:VBR
#define VIDEO2_BITRATE 		 	"video2_bitrate"	//
#define VIDEO2_QUALITY 		 	"video2_quality"        //5:普通,7:较好,9:最好
#define VIDEO2_FRAMERATE		"video2_framerate"	//1,5,10,15,20,25,30,
#define VIDEO2_IFRAMEINTERVAL           "video2_int"		//1,4,20,100,200
#define VIDEO2_RESOLUTION		"video2_resolution"	//176x144, 352x288, 320x240 640x480

#define VIDEO3_ENABLE 			"video3_enable"		//0:disable 1:enable
#define VIDEO3_DATA			"video3_data"
#define VIDEO3_CODEC 			"video3_codec"          //0:H264, 1:MJPG
#define VIDEO3_BITRATECTRL 		"video3_control"	//0:CBR,  1:VBR
#define VIDEO3_BITRATE 			"video3_bitrate"	//
#define VIDEO3_QUALITY 			"video3_quality"	//5:普通,7:较好,9:最好
#define VIDEO3_FRAMERATE 		"video3_framerate"	//1,5,10,15,20,25,30
#define VIDEO3_IFRAMEINTERVAL		"video3_int"		//1,4,20,100,200
#define VIDEO3_RESOLUTION 		"video3_resolution"	//176x144, 352x288, 320x240 640x480 1280x720


#define AUDIO1_DATA				"audio_data"

#define PTZ_MOVE_LEFT "ptz_move_left"
#define PTZ_MOVE_RIGTE "ptz_move_right"
#define PTZ_MOVE_UP "ptz_move_up"
#define PTZ_MOVE_DOWN "ptz_move_down"
#define PTZ_STOP "ptz_stop"

#define HARDWARE_ID "hardware_id"

const int kVideoSampleRate = 40;//40 ms per frame
const int kAudioSampleRate = 20;//20 ms

const int kCommandGetValue  = 101;

struct MediaControlData : public talk_base::MessageData {
  int video1;//1 to request start video1 , 0 to request stop
  int video2;//1 to request start video2, 0 to request stop
  int video3;
  int audio;//1 to request start audio, 0 to request stop
  MediaControlData(int v1,int v2,int v3 ,int a) : video1(v1),video2(v2),video3(v3), audio(a) { }
};


HisiMediaDevice::HisiMediaDevice():
  media_thread_(0),video1_handle_(0),video2_handle_(0),audio_handle_(0),
  oldNetType(-1),oldIp(-1),video3_handle_(0)
{
  int ret = Raycomm_InitParam();
  LOG(INFO)<<"Raycomm_InitParam : "<<ret;
  ret = Raycomm_MediaDataInit();
  LOG(INFO)<<"Raycomm_MediaDataInit : "<<ret;
  InitDeviceVideoInfo();
  media_thread_ = new talk_base::Thread();
  media_thread_->Start();
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
  //start get media
  media_thread_->Post(this, MSG_MEDIA_CONTROL, new MediaControlData(1,1,1,1));
  media_thread_->PostDelayed(10000,this,MSG_NET_CHECK);
  AlarmNotify::Instance()->StartNotify();
  return KeTunnelCamera::Init(client);
}

bool HisiMediaDevice::InitDeviceVideoInfo()
{
  video1_info_.frameRate = this->GetVideoFrameRate(1);
  video1_info_.frameResolution = this->GetVideoFrameType(1);
  video1_info_.frameInterval = 1000/video1_info_.frameRate;
  video2_info_.frameRate = this->GetVideoFrameRate(2);
  video2_info_.frameResolution = this->GetVideoFrameType(2);
  video2_info_.frameInterval = 1000/video2_info_.frameRate;
  video3_info_.frameRate = this->GetVideoFrameRate(3);
  video3_info_.frameResolution = this->GetVideoFrameType(3);
  video3_info_.frameInterval = 1000/video3_info_.frameRate;

  LOG(INFO)<<"video1_info_.frameInterval="<<video1_info_.frameInterval<<
             "; video2_info_.frameInterval="<<video2_info_.frameInterval<<
             "; video3_info_.frameInterval="<<video3_info_.frameInterval;
  return true;
}



void HisiMediaDevice::SendVideoFrame(const char *data, int len, int level)
{
  if(level == 1){
      SignalVideoData1(data,len);
    }else if(level == 2){
      SignalVideoData2(data,len);
    }else if( level == 3){
      SignalVideoData3(data,len);
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
  for (int i = 0;i < videoRecordList.file_num;i++) {
      Json::Value jrecord;
      jrecord["fileName"] = videoRecordList.rec_file[i].path;
      jrecord["fileDate"] = videoRecordList.rec_file[i].date;
      jrecord["fileSize"] = videoRecordList.rec_file[i].size;
      jmessage["recordList"].append(jrecord);
    }
  std::string msg = writer.write(jmessage);
  this->terminal_->SendByRouter(peer_id,msg);

}

void HisiMediaDevice::OnCommandJsonMsg(const std::string &peerId, Json::Value &jmessage)
{
  std::string command;
  bool ret = GetStringFromJsonObject(jmessage, kaerp2p::kKaerMsgCommandName, &command);
  if(!ret){
      LOG(WARNING)<<"get command error-"<<command<<" from"<<peerId;
      return;
    }
  if(command.compare("alarm_status") == 0){
      int status;
      if(GetIntFromJsonObject(jmessage,"value",&status) &&
         status != kCommandGetValue){//set value
          SetAlarmStatus(status);
        }else{//get value
          ReportAlarmStatus(peerId);
        }
    }
  if(command.compare("restart") == 0){
      LOG(INFO)<<"receive restart message ,the device will reboot";
      Raycomm_Reboot();
    }
  if(command.compare("rename")==0){
      std::string name;
      if(GetStringFromJsonObject(jmessage,"name", &name)){
          int r  = Raycomm_SetTitle(name.c_str());
          LOG(INFO)<<"receive rename message ,set device titile with "<<name<<
                     " ;result "<<r;
        }
    }
  else{
      kaerp2p::KeTunnelCamera::OnCommandJsonMsg(peerId,jmessage);
    }
}

bool HisiMediaDevice::SetAlarmStatus(int status)
{
  Raycomm_SetAlarmEnable(status);
  return true;
}

void HisiMediaDevice::ReportAlarmStatus(const std::string &peerId)
{
  int ret = Raycomm_GetAlarmEnable();
  Json::Value jmessage;
  jmessage["type"] = "tunnel";
  jmessage["command"] = "alarm_status";
  jmessage["value"] = ret;
  Json::StyledWriter writer;
  std::string msg = writer.write(jmessage);
  this->terminal_->SendByRouter(peerId,msg);
}


void HisiMediaDevice::OnMessage(talk_base::Message *msg)
{
  switch (msg->message_id) {
    case MSG_MEDIA_CONTROL:{
        MediaControlData* mcd = static_cast<MediaControlData*>(msg->pdata);
        if(mcd->video1 == 1 && video1_handle_ == 0){
            video1_handle_ =  Raycomm_ConnectMedia(VIDEO1_DATA,0);
            LOG(INFO)<<"HisiMediaDevice start video1 " <<video1_handle_;
            media_thread_->Post(this,HisiMediaDevice::MSG_SEND_VIDEO1);
          }else if(mcd->video1 == 0 && video1_handle_ != 0){
            Raycomm_DisConnectMedia(video1_handle_);
            LOG(INFO)<<"HisiMediaDevice stop video";
            video1_handle_ = 0;
          }

        if(mcd->video2 == 1 && video2_handle_ == 0){
            video2_handle_ =  Raycomm_ConnectMedia(VIDEO2_DATA,0);
            LOG(INFO)<<"HisiMediaDevice start video2 " <<video2_handle_;
            media_thread_->Post(this,HisiMediaDevice::MSG_SEND_VIDEO2);
          }else if(mcd->video2 == 0 && video2_handle_ != 0){
            Raycomm_DisConnectMedia(video2_handle_);
            LOG(INFO)<<"HisiMediaDevice stop video";
            video2_handle_ = 0;
          }

        if(mcd->video3 == 1 && video3_handle_ == 0){
            video3_handle_ =  Raycomm_ConnectMedia(VIDEO3_DATA,0);
            LOG(INFO)<<"HisiMediaDevice start video3 " <<video3_handle_;
            if(video3_handle_ > 0){
                media_thread_->Post(this,HisiMediaDevice::MSG_SEND_VIDEO3);
              }
          }else if(mcd->video3 == 0 && video3_handle_ > 0){
            Raycomm_DisConnectMedia(video3_handle_);
            LOG(INFO)<<"HisiMediaDevice stop video";
            video3_handle_ = 0;
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
    case MSG_SEND_VIDEO1:{
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
            media_thread_->Post(this,HisiMediaDevice::MSG_SEND_VIDEO1);
          }else{
            //wait and get
            media_thread_->PostDelayed(video1_info_.frameInterval,this,
                                       HisiMediaDevice::MSG_SEND_VIDEO1);
          }
        break;
      }
    case MSG_SEND_VIDEO2:{
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
            media_thread_->Post(this,MSG_SEND_VIDEO2);
          }else{
            media_thread_->PostDelayed(video2_info_.frameInterval,this,MSG_SEND_VIDEO2);
          }
        break;
      }
    case MSG_SEND_VIDEO3:{
        if(video3_handle_ == 0) break;
        unsigned int timespan;
        int media_len = Raycomm_GetMediaData(video3_handle_,media_buffer_,
                                             MEDIA_BUFFER_LENGTH,&timespan);
        if(media_len > 0){
            if(media_len == MEDIA_BUFFER_LENGTH){
                LOG(LS_ERROR)<<"Raycomm_GetMediaData--- video3 "<<
                               "no enough buffer for this large frame";
              }
            this->SendVideoFrame(media_buffer_,media_len,3);
            //get next frame
            media_thread_->Post(this,MSG_SEND_VIDEO3);
          }else{
            media_thread_->PostDelayed(video3_info_.frameInterval,this,MSG_SEND_VIDEO3);
          }
        break;
      }
    case MSG_SEND_AUDIO:{
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
    case MSG_NET_CHECK:{
        this->CheckNetStatus();
        media_thread_->PostDelayed(10000,this,MSG_NET_CHECK);
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
    }else if(level == 3){
      key = VIDEO3_RESOLUTION;
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
    }else if(level == 3){
      key = VIDEO3_FRAMERATE;
    }
  Raycomm_GetParam(key,buf,0);
  int frameRate = atoi(buf);
  //LOG(INFO)<<"HisiMediaDevice::GetVideoFrameRate---"<<frameRate;
  return frameRate;
}

void HisiMediaDevice::CheckNetStatus()
{
  //    LOG(INFO)<<"HisiMediaDevice::CheckNetStatus---oldip"<<oldIp<<
  //               " oldNet"<<oldNetType;
  int ip = Raycomm_GetIP();
  int net = Raycomm_GetNetType();
  if (ip != oldIp && oldIp != -1) {
      LOG(INFO)<<"HisiMediaDevice::CheckNetStatus---"<<
                 "Ip changed new ip is "<<ip <<", old ip is "<<oldIp;
      this->SignalNetStatusChange();
    } else if( net != oldNetType && oldNetType != -1 ) {
      LOG(INFO)<<"HisiMediaDevice::CheckNetStatus---"<<
                 "net changed new net is "<<net<<" old net is "<<oldNetType;
      this->SignalNetStatusChange();
    }
  oldIp = ip;
  oldNetType = net;
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
    }
  else if(level == 2){
      *info = this->video2_info_;
    }
  else if(level == 3){
      *info = this->video3_info_;
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
  if (snapsize > 0) {
      talk_base::Base64::EncodeFromArray(snapbuf,snapsize,&picBase64Data);
    }
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
