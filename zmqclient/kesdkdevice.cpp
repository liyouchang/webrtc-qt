#include "kesdkdevice.h"


//#include <regex.h>
#include <time.h>
#include <algorithm>
#include <iostream>
#include <errno.h>


#include "talk/base/logging.h"
#include "talk/base/stringutils.h"
#include "talk/base/bind.h"
#include "talk/base/thread.h"
#include "talk/base/buffer.h"
#include "talk/base/stringencode.h"
#include "talk/base/base64.h"
#include "talk/base/nethelpers.h"
#include "talk/base/socketaddress.h"

#include "keapi/common_api.h"
#include "keapi/media_api.h"
#include "keapi/store_api.h"
#include "keapi/web_api.h"
#include "keapi/alarm_api.h"
#include "keapi/platform_api.h"
#include "keapi/unicode.h"


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

const int kNullStreamHandle = -1;
const int kCheckStreamDelay = 60000;
const int kCheckNetDelay = 5000;
enum StreamLevel{
    kLevelNoneStream = 0,
    kLevelMainStream = 1,
    kLevelSubStream = 2,
    kLevelExtStream = 3,
    kLevelAudioStream = 4
};

int ResoToFramType(int resoValue){
    switch(resoValue){
    case RESO_CIF:return kaerp2p::kFrameCIF;
    case RESO_D1:return kaerp2p::kFrameD1;
    case RESO_720P:return kaerp2p::kFrame720P;
    case RESO_VGA: return kaerp2p::kFrameVGA;
    default:
        LOG_F(WARNING)<<"the camera reso is not support "<<resoValue;
        return 0;
    }
}

int netAppCallBack(void * pData)
{
    LOG_F(INFO) <<" net app call back "<<pData;
    return 0;
}

KeSdkDevice::KeSdkDevice():video1_handle_(kNullStreamHandle),
    video2_handle_(kNullStreamHandle),video3_handle_(kNullStreamHandle),
    audio_handle_(kNullStreamHandle),oldIp(-1)
{
    deviceThread = talk_base::Thread::Current();

    zmqThread = new talk_base::Thread();
    zmqThread->Start();

    try{
        //        context_ = new zmq::context_t(1,5);
        zmqContext = new zmq::context_t(1,1);
        repSocket = new zmq::socket_t(*zmqContext,ZMQ_REP);
        repSocket->bind("tcp://*:22555");
        //        int reconnectInterval = 10000;
        //        repSocket->setsockopt(ZMQ_RECONNECT_IVL,&reconnectInterval,sizeof(reconnectInterval));
        int highwater = 100;
        repSocket->setsockopt(ZMQ_SNDHWM,&highwater,sizeof(highwater));
        repSocket->setsockopt(ZMQ_RCVHWM,&highwater,sizeof(highwater));
        int ligger = 100;
        repSocket->setsockopt(ZMQ_LINGER,&ligger,sizeof(ligger));
    }catch(zmq::error_t e){
        LOG_F(WARNING) <<" failed , enum:"<<e.num()<<" edes:" <<e.what();
    }

    zmqThread->Post(this,MSG_ZMQ_RECV);
    CONFIG_Initialize();
    gpio_handle = GPIO_Open("/dev/hi_gpio");
    clock_handle = CLOCK_Open(CLOCK_TYPE_HIRTC);

    SYSTEM_Initialize();
    NET_Initialize();
    ntp_chandle = CLOCK_Open(CLOCK_TYPE_NTP);
    MEDIA_Initialize();

    STORE_Initialize();
    ALARM_Initialize();

#ifndef NOT_USE_WEB
    WEB_Initialize();
#endif

    PLATFORM_Tools_Open();
    PLATFORM_Update_Open();
}

KeSdkDevice::~KeSdkDevice()
{
    // zmq 的线程不要终止,否则会阻塞
    //    LOG_F(INFO) << "destroy device";
    //    zmqThread->Quit();
    //    try{
    //        LOG_F(INFO) << "delete zmq socket";
    //        delete repSocket;
    //        repSocket = NULL;
    //        LOG_F(INFO) << "delete zmq context";
    //        delete zmqContext;
    //        zmqContext = NULL;
    //    }catch(zmq::error_t e){
    //        LOG_F(WARNING) <<" failed , enum:"<<e.num()<<" edes:" <<e.what();
    //    }
    //    LOG_F(INFO) << "delete zmq thread";
    //    delete zmqThread;

    if( video1_handle_ != kNullStreamHandle){
        FIFO_Stream_Close(video1_handle_);
        video1_handle_ = kNullStreamHandle;
    }
    if(video2_handle_ != kNullStreamHandle){
        FIFO_Stream_Close(video2_handle_);
        video2_handle_ = kNullStreamHandle;
    }
    if(video3_handle_ != kNullStreamHandle){
        FIFO_Stream_Close(video3_handle_);
        video3_handle_ = kNullStreamHandle;
    }
    if(audio_handle_ != kNullStreamHandle){
        FIFO_Stream_Close(audio_handle_);
        audio_handle_ = kNullStreamHandle;
    }

    PLATFORM_Update_Close();

    PLATFORM_Tools_Close();
#ifndef NOT_USE_WEB
    WEB_Cleanup();
#endif

    ALARM_Cleanup();
    STORE_Cleanup();

    MEDIA_Cleanup();
    CLOCK_Close(ntp_chandle);
    NET_Cleanup();

    SYSTEM_Cleanup();

    CLOCK_Close(clock_handle);

    GPIO_Close(gpio_handle);
    CONFIG_Cleanup();
}

bool KeSdkDevice::Init(kaerp2p::PeerTerminalInterface *t)
{
    InitVideoInfo();
    RegisterCallBack::Instance()->SignalVideoFrame.connect(
                this,&KeSdkDevice::SendVideoFrame);
    RegisterCallBack::Instance()->SignalAudioFrame.connect(
                this,&KeSdkDevice::SendAudioFrame);
    RegisterCallBack::Instance()->SignalReboot.connect(
                this,&KeSdkDevice::QuitMainThread);


    oldIp = talk_base::NetworkToHost32(NET_Get_RouteIP(NULL));
    LOG_F(INFO) << "get old ip is "<<oldIp;
    //    deviceThread->PostDelayed(kCheckStreamDelay,this,MSG_CheckCloseStream);
    deviceThread->PostDelayed(kCheckNetDelay,this,MSG_NET_CHECK);

    return KeTunnelCamera::Init(t);
}

void KeSdkDevice::GetCameraVideoInfo(int level, kaerp2p::VideoInfo *info)
{
    //每次都初始化设备参数,以防止在其他地方(web)改变了视频的格式
    InitVideoInfo();

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

void KeSdkDevice::OnTunnelOpened(kaerp2p::PeerTerminalInterface *t,
                                 const std::string &peer_id)
{
    ASSERT(terminal_ == t);
    //    LOG(INFO)<<"KeSdkDevice::OnTunnelOpened---"<<peer_id;
    KeSdkProcess *process = new KeSdkProcess(peer_id,this);
    this->AddMsgProcess(process);
}
class TalkPacket : public talk_base::MessageData {
 public:
  TalkPacket(const char* data, size_t size, int timespan)
        : timespan(timespan), talkData(data,size){
  }
  talk_base::Buffer talkData;
  int timespan;
};

void KeSdkDevice::OnRecvTalkData(const std::string &peer_id, const char *data, int len)
{
    KEFrameHead * head = (KEFrameHead *)data;
    int nowTime = talk_base::Time();
    LOG_T_F(INFO)<<" talk time is "<< head->second*1000 + head->millisecond*10<<
                   " now time is "<< nowTime;
//    const int nalLen = 4;
    int dataPos =  sizeof(KEFrameHead);
    if(head->frameLen == len - dataPos){
        //放入另外的线程播放声音,以防止在数据接受线程中阻塞.若阻塞会产生异常
        TalkPacket *talkData = new TalkPacket(data+dataPos,head->frameLen,nowTime);
        deviceThread->Post(this,MSG_RECV_TALK,talkData);
    }else{
        LOG_F(WARNING)<<"tal from "<<peer_id<<" frame format error";
    }

}

const int kCommandGetValue  = 101;

void KeSdkDevice::OnCommandJsonMsg(const Json::Value &jmessage,Json::Value * jresult)
{
    std::string command;
    bool ret = GetStringFromJsonObject(jmessage,kaerp2p::kKaerMsgCommandName, &command);
    if(!ret){
        LOG(WARNING)<<"get command error-"<<command<<" from" ;
        return;
    }
    if(command.compare("ptz") == 0){
        std::string ptz_control;
        GetStringFromJsonObject(jmessage,"control",&ptz_control);
        int param;
        GetIntFromJsonObject(jmessage,"param",&param);
        this->SetPtz(ptz_control,param);
    }
    else if(command.compare("query_record") == 0){
        Json::Value jcondition;
        if(!GetValueFromJsonObject(jmessage, "condition", &jcondition))
        {
            LOG(WARNING)<<"get query_record value error from" ;
            return;
        }
        int totalNum = 0;
        Json::Value jrecordList(Json::arrayValue);
        bool result =  QueryRecord(jcondition,&jrecordList,&totalNum);

        *jresult = jmessage;
        (*jresult)["result"] = result;
        (*jresult)["totalNum"] = totalNum;
        (*jresult)["recordList"] = jrecordList;
        //        this->ReportJsonMsg(peerId,jmessage);
    }
    else if( command.compare("wifi_info") == 0 ){
        Json::Value jwifi = GetWifiJsonArray();
        (*jresult)["type"] = "tunnel";
        (*jresult)["command"] = "wifi_info";
        (*jresult)["wifis"] = jwifi;
        //        this->ReportJsonMsg(peerId,jresult);
    }
    else if( command.compare("set_wifi") == 0 ){
        Json::Value jwifiParam;
        if(!GetValueFromJsonObject(jmessage, "param", &jwifiParam)){
            LOG(WARNING)<<"get set_wifi value error from";
            return;
        }
        bool ret = this->SetWifiInfo(jwifiParam);
        (*jresult) = this->GetResultMsg(command,ret);
    }
    else if( command.compare("wifi_status") == 0 ) {
        int wifiStatus = Net_Get_WifiLink_Status();
        (*jresult) = jmessage;
        (*jresult)["status"] = wifiStatus;
        LOG_F(INFO) <<"get wifi status "<< wifiStatus;
        //        this->ReportJsonMsg(peerId,jmessage);
    }
    else if( command.compare("rename") == 0 ) {
        std::string name;
        if(!GetStringFromJsonObject(jmessage,"name", &name)){
            LOG_F(WARNING) <<" receive rename msg error ";
            return;
        }
        bool r  = SetOsdTitle(name);
        LOG(INFO)<<"receive rename message ,set device title - "<<name<<
                   " ;result "<<r;
        (*jresult) = this->GetResultMsg(command,ret);
    }
    else if( command.compare("arming_status") == 0 ){
        int status;
        if( GetIntFromJsonObject(jmessage,"value",&status) &&
                status != kCommandGetValue ) {//set value
            SetArmingStatus(status);
        }
        (*jresult) = jmessage;
        (*jresult)["value"] = this->GetArmingStatus();
    }
    else if( command.compare("reboot") == 0 ) {
        *jresult = this->GetResultMsg(command,ret);
        //        this->QuitMainThread();
        SYSTEM_Set_Reboot();
    }
    else{
        kaerp2p::KeTunnelCamera::OnCommandJsonMsg(jmessage,jresult);
    }
}

void KeSdkDevice::OnMessage(talk_base::Message *msg)
{
    switch (msg->message_id) {
    case MSG_CheckCloseStream:
        CheckCloseStream_d();
        deviceThread->PostDelayed(kCheckStreamDelay,this,MSG_CheckCloseStream);
        break;
    case MSG_MEDIA_CONTROL:{
        talk_base::TypedMessageData<int>* mcd =
                static_cast<talk_base::TypedMessageData<int>*>(msg->pdata);
        this->MediaStreamOpen_d(mcd->data());
        delete mcd;
    }
        break;
    case MSG_NET_CHECK:
        CheckNetIp_d();
        //        RegisterCallBack::Instance()->SignalTerminalAlarm(6,"test alarm","");
        deviceThread->PostDelayed(kCheckNetDelay,this,MSG_NET_CHECK);
        break;
    case MSG_ZMQ_RECV:
        ZmqRepMsg_z();
        if(repSocket){
            zmqThread->Post(this,MSG_ZMQ_RECV);
        }
        break;
    case MSG_RECV_TALK:{
        TalkPacket * msgData = static_cast<TalkPacket *>(msg->pdata);
        LOG_F(INFO)<<" talk "<<msgData->talkData.length();
        MEDIA_Audio_Talk(const_cast<char *>(msgData->talkData.data()),msgData->talkData.length());
        delete msgData;
    }
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
            LOG_F(LS_VERBOSE)<<"send video "<< camProcess->peer_id();
        }
    }
}

void KeSdkDevice::SendAudioFrame(const char *data, int len)
{
    talk_base::CritScope cs(&crit_);
    for( int i = 0 ; i < processes_.size() ; i++ ){
        kaerp2p::KeMessageProcessCamera * camProcess =
                static_cast<kaerp2p::KeMessageProcessCamera *>(processes_[i]);
        if(camProcess->audio_status == 1){
            camProcess->OnAudioData(data,len);
        }
    }
}

void KeSdkDevice::CheckCloseStream_d()
{
    int mainStreamCount = 0;
    int subStreamCount = 0;
    int extStreamCount = 0;
    int audioCount = 0;

    {
        talk_base::CritScope cs(&crit_);
        for(int i = 0 ; i < processes_.size() ; i++) {
            kaerp2p::KeMessageProcessCamera * camProcess =
                    static_cast<kaerp2p::KeMessageProcessCamera *>(processes_[i]);
            if(camProcess->video_status == kLevelMainStream){
                ++mainStreamCount;
            }else if(camProcess->video_status == kLevelSubStream){
                ++subStreamCount;
            }else if(camProcess->video_status == kLevelExtStream){
                ++extStreamCount;
            }
            if(camProcess->audio_status == 1){
                ++audioCount;
            }
        }
    }

    LOG_F(INFO)<<"count main "<<mainStreamCount<<" sub "<<subStreamCount<<
                 " ext "<<extStreamCount<<" audio "<<audioCount;
    if(mainStreamCount == 0 && video1_handle_ != kNullStreamHandle){
        LOG_F(INFO) << " close main video " << video1_handle_;
        FIFO_Stream_Close(video1_handle_);
        video1_handle_ = kNullStreamHandle;
    }
    if(subStreamCount == 0 && video2_handle_ != kNullStreamHandle){
        LOG_F(INFO) << " close sub video " << video2_handle_;
        FIFO_Stream_Close(video2_handle_);
        LOG_F(INFO) << "sub video closed";

        video2_handle_ = kNullStreamHandle;
    }
    if(extStreamCount == 0 && video3_handle_ != kNullStreamHandle){
        LOG_F(INFO) << " close ext video " << video3_handle_;
        FIFO_Stream_Close(video3_handle_);
        video3_handle_ = kNullStreamHandle;
    }
    if(audioCount == 0 && audio_handle_ != kNullStreamHandle){
        LOG_F(INFO) << " close audio " << audio_handle_;
        FIFO_Stream_Close(audio_handle_);
        audio_handle_ = kNullStreamHandle;
    }
}

void KeSdkDevice::CheckNetIp_d()
{
    //在程序开始的一段时间内,获得的ip会是0;在网络切换的一段时间内获得的ip会是0
    int newIp  = NET_Get_RouteIP(NULL);
    newIp = talk_base::NetworkToHost32(newIp);
    if( newIp != oldIp){
        LOG_F(INFO)<<"ip changed new ip is "<<talk_base::SocketAddress::IPToString(newIp)<<
                     " and old ip is "<<talk_base::SocketAddress::IPToString(oldIp);
        this->SignalNetStatusChange();
        oldIp = newIp;
    }
}

bool KeSdkDevice::SetOsdTitle(const std::string &title)
{
    TITLEOSD osdTitle;
    CONFIG_Get(CONFIG_TYPE_OSDTITLE,(void *)&osdTitle);	//获取某类参数
    u2g(osdTitle.Contert,(const unsigned char *)title.c_str());
    //    memcpy(osdTitle.Contert,title.c_str(),title.length());
    CONFIG_Set(CONFIG_TYPE_OSDTITLE,(void *)&osdTitle);
    return true;
}

//level 1~4 , 1-video1 ,2-video2,3-video3,4-audio stream
void KeSdkDevice::MediaStreamOpen(int level)
{
    talk_base::TypedMessageData<int> * data = new talk_base::TypedMessageData<int>(level);
    deviceThread->Post(this,MSG_MEDIA_CONTROL,data);
}

void KeSdkDevice::MediaGetIDR(int level)
{
    if(level != kLevelMainStream && level != kLevelSubStream &&
            level != kLevelExtStream)
    {
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
    LOG_F(INFO) << "local ip is " << (int)net.localIP[0] << "."<<(int)net.localIP[1]<<
                   "."<<(int)net.localIP[2]<<"."<<(int)net.localIP[3];
    std::string ret((const char *)net.macAddress);
    LOG_F(INFO)<<"get mac id is "<<ret;
    return ret;
}

std::string KeSdkDevice::GetTerminalType()
{
    char devModel[64];
    SYSTEM_Get_Model(devModel);
    return devModel;
}

void KeSdkDevice::SetNtp(const std::string &ntpIp, int port, const std::string &zone)
{
    int iNetIp = talk_base::SocketAddress::StringToIP(ntpIp);

    int zonesecond = 0*60*60;
    if( zone.size() > 2 ){
        int pos = zone.find(':');
        if(pos != std::string::npos){
            int hour = atoi(zone.substr(1,pos).c_str());
            int minute = atoi(zone.substr(pos+1).c_str());
            if(zone[0] == '+'){
                zonesecond = hour*60*60 + minute*60;
            } else if (zone[0] == '-') {
                zonesecond = -(hour*60*60 + minute*60);
            } else {
                LOG_F(WARNING)<<" zone format error";
            }
        } else {
            LOG_F(WARNING)<<" zone format error";
        }
    }
    //zone 时区转化成秒数
    CLOCK_Set_NTP(iNetIp,port,zonesecond);
}


void KeSdkDevice::InitVideoInfo()
{
    struct MEDIAPARAM media;
    CONFIG_Get(CONFIG_TYPE_MEDIA,(void *)&media);
    //    printf("resolution %d frame_rate %d rate_ctrl_mode %d bitrate %d piclevel %d\n",
    //           media.main[0].resolution,media.main[0].frame_rate,
    //            media.main[0].rate_ctrl_mode,media.main[0].bitrate,media.main[0].piclevel);
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

void KeSdkDevice::ZmqRepMsg_z()
{
    zmq::message_t zmsg;
    try{
        //TODO: 使用非阻塞模式接收数据
        repSocket->recv(&zmsg);
    }catch(zmq::error_t e){
        LOG_F(WARNING) <<" failed , enum:"<<e.num()<<" edes:" <<e.what();
        return;
    }

    std::string strMsg((char *)zmsg.data(),zmsg.size());
    Json::Reader reader;
    LOG_F(INFO)<<"rep msg "<<strMsg;
    Json::Value jmessage;
    if (!reader.parse(strMsg, jmessage)) {
        LOG(WARNING) << "Received unknown message. ";
        return;
    }
    Json::Value jresult;
    this->OnCommandJsonMsg(jmessage,&jresult);
    if( !jresult.isNull() ) {
        Json::StyledWriter writer;
        strMsg = writer.write(jresult);
        LOG(LS_VERBOSE) << "send msg is " << strMsg;
        repSocket->send(strMsg.c_str(),strMsg.length());
    }
}

bool KeSdkDevice::SetPtz(std::string control, int param)
{
    const int kDefaultSpeed  = 63;
    int cmd = 0;
    int speed = kDefaultSpeed;
    if(control.compare("stop") == 0){
        cmd = 0;
    }else if(control.compare("move_left") == 0){
        cmd = 3;
        speed = param;
    }else if(control.compare("move_right") == 0){
        cmd = 4;
        speed = param;
    }else if(control.compare("move_up") == 0 ){
        cmd = 1;
        speed = param;
    }else if(control.compare("move_down") == 0){
        cmd = 2;
        speed = param;
    }else{
        LOG_F(WARNING)<<" wrong ptz control";
        return false;
    }
    //
    int ret = MOTOR_Control(0,cmd,kDefaultSpeed,param);
    LOG_F(LS_VERBOSE)<<" Control_MOTOR result "<< ret;
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
    LOG_F(INFO)<<"NET_Set_Wifi --- enable "<< wifiParam.enable<<"  ssid "<<wifiParam.ssid;

    int ret = NET_Set_Wifi(&wifiParam);
    return true;
}

bool StringToClock(std::string strTime,st_clock_t * time)
{
    struct tm tm_t;
    if(strptime(strTime.c_str(),"%Y/%m/%d %H:%M:%S", &tm_t) == NULL){
        return false;
    }
    time->year = tm_t.tm_year + 1900;
    time->month = tm_t.tm_mon + 1;
    time->day = tm_t.tm_mday;
    time->hour  = tm_t.tm_hour;
    time->minute = tm_t.tm_min;
    time->second = tm_t.tm_sec;
    return true;
}
std::string ClockToString(st_clock_t  time)
{
    char timebuf[64];
    //2014/04/17 17:51:00
    sprintf(timebuf,"%04d/%02d/%02d %02d:%02d:%02d",
            time.year,time.month,time.day,time.hour,time.minute,time.second);
    return timebuf;
}

bool KeSdkDevice::QueryRecord(Json::Value jcondition, Json::Value *jrecordList, int *totalNum)
{    
    std::string startTime,endTime;
    int offset,toQuery;

    if( !GetStringFromJsonObject(jcondition,"startTime",&startTime) ||
            !GetStringFromJsonObject(jcondition,"endTime",&endTime) ||
            ! GetIntFromJsonObject(jcondition,"offset",&offset) ||
            !GetIntFromJsonObject(jcondition,"toQuery",&toQuery))
    {
        LOG_F(WARNING)<<"parse condition error";
        return false;
    }

    st_clock_t startClock,endClock;
    if(!StringToClock(startTime,&startClock) || !StringToClock(endTime,&endClock))
    {
        LOG_F(WARNING)<<"input time  format error";
        return false;
    }
    int list_num  = STORE_Get_File_List(&startClock,&endClock,0,STORE_TYPE_PLAN,0,NULL);

    if(list_num <= 0){
        *totalNum = 0;
        LOG_F(WARNING)<<"No record found";
        return true;
    }
    LOG_F(INFO)<<"get num "<<list_num;
    *totalNum = list_num;
    if(offset > list_num ) {
        LOG_F(WARNING)<<" offset is large than total num ";
        return false;
    }
    st_store_list_t * stList  = new st_store_list_t[list_num];

    STORE_Get_File_List(&startClock,&endClock,0,STORE_TYPE_PLAN,list_num,stList);

    int copyNum = std::min(list_num-offset,toQuery);

    for(int i = offset ; i < offset + copyNum ; i++) {
        Json::Value jrecord;
        jrecord["fileName"] = stList[i].filePath;
        jrecord["fileEndTime"] = ClockToString(stList[i].stEndTime);
        jrecord["fileSize"] = stList[i].iFileSize;
        jrecordList->append(jrecord);
    }
    delete [] stList;
    return true;
}

bool KeSdkDevice::SetArmingStatus(int status)
{
    LOG_F(INFO) << status;
    if(status == 1){
        ALARM_MD_Defense(0);
    }else if(status == 0){
        ALARM_MD_UnDefense(0);
    }else{
        LOG_F(WARNING)<<"status param error";
        return false;
    }
    return true;
}

int KeSdkDevice::GetArmingStatus()
{
    return  ALARM_Get_Defense_Status(FIFO_ALARM_MV,0,0);
}

void KeSdkDevice::QuitMainThread()
{
    this->deviceThread->Quit();
    LOG_F(WARNING) << " quit main thread";
}

bool KeSdkDevice::TalkAvaliable()
{
    int isTalking = MEDIA_Audio_isTalking();
    LOG_F(INFO) << isTalking;
    if(isTalking == 1 ){
        return false;
    }else {
        return true;
    }
}

void KeSdkDevice::SetNetInfo()
{
    struct NETPARAM	net;
    CONFIG_Register_Callback(CONFIG_TYPE_NET,netAppCallBack);
    CONFIG_Get(CONFIG_TYPE_NET,(void *)&net);
    //    printf("localIP:%d.%d.%d.%d\n",net.localIP[0],net.localIP[1],net.localIP[2],net.localIP[3]);
    LOG_F(INFO)<<" dhcp enable is "<< (int)net.dhcpEnable;
    //    net.localIP[0] = 10;
    //    net.localIP[1] = 10;
    //    net.localIP[2] = 0;
    //    net.localIP[3] = 212;
    //    net.gateIP[0]    = 10;
    //    net.gateIP[1]    = 10;
    //    net.gateIP[2]    = 0;
    //    net.gateIP[3]    = 1;
    //    net.netMask[0]   = 255;
    //    net.netMask[1]   = 255;
    //    net.netMask[2]   = 255;
    //    net.netMask[3]   = 0;
    net.dhcpEnable = 1;
    //    memcpy(net.username,"system",8);
    //    memcpy(net.password,"123456",8);
    CONFIG_Set(CONFIG_TYPE_NET,(void *)&net);
    //        CONFIG_Get(CONFIG_TYPE_NET,(void *)&net);
    //        printf("localIP:%d.%d.%d.%d  username:%s  password:%s\n",net.localIP[0],net.localIP[1],net.localIP[2],net.localIP[3],net.username,net.password);

}



KeSdkDevice::RegisterCallBack::RegisterCallBack()
{
    FIFO_Register_Callback(FIFO_H264_MAIN,&RegisterCallBack::MainStreamCallBack);
    FIFO_Register_Callback(FIFO_H264_SUB,&RegisterCallBack::SubStreamCallBack);
    FIFO_Register_Callback(FIFO_H264_EXT,&RegisterCallBack::ExtStreamCallBack);
    FIFO_Register_Callback(FIFO_H264_AUDIO,&RegisterCallBack::AudioStreamCallBack);
    SYSTEM_Register_Callback(&RegisterCallBack::RebootCallback);
    ALARM_Register_Callback(&RegisterCallBack::AlarmCallback);
}

KeSdkDevice::RegisterCallBack *KeSdkDevice::RegisterCallBack::Instance(){
    static RegisterCallBack instance;
    return &instance;
}

int KeSdkDevice::RegisterCallBack::MainStreamCallBack(char *pFrameData, int iFrameLen)
{
    LOG(LS_VERBOSE) <<" MainStreamCallBack ---"<< iFrameLen;
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

int KeSdkDevice::RegisterCallBack::RebootCallback()
{
    LOG(WARNING) << " call back reboot call back, the process will end ";
    RegisterCallBack::Instance()->SignalReboot();
}

int KeSdkDevice::RegisterCallBack::AlarmCallback(st_alarm_upload_t *alarmInfo, char *pJpegData, int iJpegLen)
{
    LOG(INFO) << "alrm call back "<<alarmInfo->enAlarm << "; picture lenght "<<iJpegLen;

    std::string picBase64Data;
    if (iJpegLen > 0) {
        talk_base::Base64::EncodeFromArray(pJpegData,iJpegLen,&picBase64Data);
    }

    char utf8Result[256] = {0};
    if(g2u((unsigned char *)utf8Result,(unsigned char *)alarmInfo->cInfo) == 0){//error
        LOG_F(WARNING)<<" alarm info convert error";
    }
    RegisterCallBack::Instance()->SignalTerminalAlarm(alarmInfo->enAlarm,utf8Result,
                                                      picBase64Data);

    //    Json::StyledWriter writer;
    //    Json::Value jmessage;
    //    jmessage["type"] = "Terminal_Alarm";
    //    jmessage["MAC"] = mac_;
    //    jmessage["AlarmType"] = alarmType;
    //    jmessage["AlarmInfo"] = alarmInfo;
    //    jmessage["Picture"] = picture;
    //    jmessage["DateTime"] = kaerp2p::GetCurrentDatetime("%F %T");
    //    std::string msg = writer.write(jmessage);
    //    this->SendToPeer(alarmServer,msg);
    //    LOG_F(INFO)<<" send alarm "<< alarmType <<" msg size "<< msg.size() <<
    //                 " alarm time "<< kaerp2p::GetCurrentDatetime("%F %T");

    return 0;
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
    LOG_F(INFO)<<"KeMessageProcessCamera::ConnectMedia--- video "<<video<<
                 " listen-"<<audio<<" talk-"<<talk<<"; frameResolution="<<
                 this->videoInfo_.frameResolution<<" ;framerate="<<videoInfo_.frameRate;

    if( video == 0 ) {//stop
        video_status = video;
    }
    else if( 0 == video_status && video != MEDIA_NOCHANGE ) {
        this->RespAskMediaReq(this->videoInfo_);
        video_status = video;
        //try to open stream
        camera->MediaStreamOpen(video);
        camera->MediaGetIDR(video);
    }else if(video != MEDIA_NOCHANGE){ //video > 0
        camera->MediaGetIDR(video);
    }

    if (audio == 0) {
        audio_status = 0;
    }
    else if (0 == audio_status && audio != MEDIA_NOCHANGE) {
        audio_status = audio;
        camera->MediaStreamOpen(kLevelAudioStream);
    }

    if(talk == 0) {
        this->SignalRecvTalkData.disconnect(camera);
        talk_status = 0;
    }
    else if(0 == talk_status && talk != MEDIA_NOCHANGE) {
        if(!camera->TalkAvaliable()){
            LOG_F(INFO) <<" talk is in use , not avaliable for now ";
            ReportMediaStatus(MEDIA_NOCHANGE,MEDIA_NOCHANGE,2);
        } else {
            this->SignalRecvTalkData.connect(camera,&KeSdkDevice::OnRecvTalkData);
            talk_status = talk;
        }
    }
    ReportMediaStatus(video_status,audio_status,talk_status);
}

void KeSdkDevice::MediaStreamOpen_d(int level)
{
    switch (level) {
    case kLevelMainStream:
        if(video1_handle_ == kNullStreamHandle){
            video1_handle_ = FIFO_Stream_Open(FIFO_STREAM_H264,0,0,0);
            LOG_F(INFO)<<"open video1 stream " << video1_handle_;
        }
        break;
    case kLevelSubStream:
        if( video2_handle_ == kNullStreamHandle){
            video2_handle_ = FIFO_Stream_Open(FIFO_STREAM_H264,0,1,0);
            LOG_F(INFO)<<"open video2 stream"<<video2_handle_;
        }
        break;
    case kLevelExtStream:
        if( video3_handle_ == kNullStreamHandle){
            video3_handle_ = FIFO_Stream_Open(FIFO_STREAM_H264,0,2,0);
            LOG_F(INFO)<<"open video3 stream"<<video3_handle_;
        }
        break;
    case kLevelAudioStream:
        if( audio_handle_ == kNullStreamHandle ){
            audio_handle_ = FIFO_Stream_Open(FIFO_STREAM_AUDIO,0,0,0);
            LOG_F(INFO)<<"open audio stream"<<audio_handle_;
        }
        break;
    default:
        LOG_F(WARNING)<<"error level "<<level;
        break;
    }

}
