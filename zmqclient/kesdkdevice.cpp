#include "kesdkdevice.h"

#include "talk/base/logging.h"
#include "libjingle_app/jsonconfig.h"

#include "keapi/common_api.h"
#include "keapi/common_define.h"
#include "keapi/media_api.h"

#include "libjingle_app/defaults.h"
//enum FramTypes{
//    FRAM_D1 = 0,
//    FRAM_QCIF = 1,
//    FRAM_CIF = 2,
//    FRAM_HD1 = 3,
//    FRAM_QVGA = 4,
//    FRAM_VGA = 7,
//    FRAM_720P = 10
//};

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


KeSdkDevice::KeSdkDevice()
{
    CONFIG_Initialize();
    struct MEDIAPARAM media;
    CONFIG_Get(CONFIG_TYPE_MEDIA,(void *)&media);
    printf("resolution %d frame_rate %d rate_ctrl_mode %d bitrate %d piclevel %d\n",
            media.main[0].resolution,media.main[0].frame_rate,
            media.main[0].rate_ctrl_mode,media.main[0].bitrate,
            media.main[0].piclevel);

    media.main[0].frame_rate = 25;
    media.main[0].resolution = RESO_720P;
    media.main[0].bitrate = 512;
    media.main[0].piclevel = 0;
    media.minor[0].frame_rate = 25;
    media.minor[0].resolution = RESO_D1;
    media.minor[0].bitrate = 256;
    media.minor[0].piclevel = 0;

    CONFIG_Set(CONFIG_TYPE_MEDIA,(void *)&media);
    CONFIG_Get(CONFIG_TYPE_MEDIA,(void *)&media);
    printf("resolution %d frame_rate %d rate_ctrl_mode %d bitrate %d piclevel %d\n",media.main[0].resolution,media.main[0].frame_rate,media.main[0].rate_ctrl_mode,media.main[0].bitrate,media.main[0].piclevel);


    video1_info_.frameRate = media.main[0].frame_rate;
    video1_info_.frameResolution = ResoToFramType(media.main[0].resolution);
    video1_info_.frameInterval = 1000/video1_info_.frameRate;
    video2_info_.frameRate = media.minor[0].frame_rate;
    video2_info_.frameResolution = ResoToFramType(media.minor[0].resolution);
    video2_info_.frameInterval = 1000/video2_info_.frameRate;
    video3_info_.frameRate = 25;
    video3_info_.frameResolution = kaerp2p::kFrameCIF;
    video3_info_.frameInterval = 1000/video3_info_.frameRate;

    MEDIA_Initialize();
    LOG_F(INFO)<<"media init";
}

KeSdkDevice::~KeSdkDevice()
{
    MEDIA_Cleanup();
    CONFIG_Cleanup();
}

bool KeSdkDevice::Init(kaerp2p::PeerTerminalInterface *t)
{

    RegisterCallBack::Instance()->SignalVideoFrame.connect(
                this,&KeSdkDevice::SendVideoFrame);
    RegisterCallBack::Instance()->SignalAudioFrame.connect(
                this,&KeSdkDevice::SendAudioFrame);
 //   LOG_F(INFO)<<"stream open";
    video1_handle_ = FIFO_Stream_Open(FIFO_STREAM_H264,0,0);
    video2_handle_ = FIFO_Stream_Open(FIFO_STREAM_H264,0,1);
    video3_handle_ = FIFO_Stream_Open(FIFO_STREAM_H264,0,2);
    audio_handle_ = FIFO_Stream_Open(FIFO_STREAM_AUDIO,0,0);

 //   LOG_F(INFO)<<"init end";

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

}

void KeSdkDevice::OnCommandJsonMsg(const std::string &peerId, Json::Value &jmessage)
{
    kaerp2p::KeTunnelCamera::OnCommandJsonMsg(peerId,jmessage);
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
   // LOG_F(INFO)<<" recevie frame "<<iFrameLen;

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
    }
    if(audio == 0){
        audio_status = 0;
    }
    else if(0 == audio_status){
        audio_status = audio;
    }
    if(talk == 0){
        talk_status = 0;
    }else if(0 == talk_status){
        talk_status = talk;
    }

}
