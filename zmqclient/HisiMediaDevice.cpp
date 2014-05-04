#include "HisiMediaDevice.h"

#include "talk/base/bind.h"
#include "talk/base/thread.h"
#include "talk/base/timeutils.h"
#include "talk/base/buffer.h"
#include "talk/base/logging.h"

#include "keapi/keapi.h"
#include "libjingle_app/KeMessage.h"
#include "libjingle_app/KeMsgProcess.h"
#include "jsonconfig.h"
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

    int ret = Raycomm_InitParam();
    LOG(INFO)<<"Raycomm_InitParam : "<<ret;

    ret = Raycomm_MediaDataInit();
    LOG(INFO)<<"Raycomm_MediaDataInit : "<<ret;

    LOG(INFO)<<"vidoe1 frame type " << GetVideoFrameType(1);
    LOG(INFO)<<"vidoe2 frame type " << GetVideoFrameType(2);

    media_thread_ = new talk_base::Thread();
    media_thread_->Start();

    //start get media
    media_thread_->Post(this,MSG_MEDIA_CONTROL,new MediaControlData(1,1,1));


    KeTunnelCamera::Init(client);
}



void HisiMediaDevice::SendVideoFrame(const char *data, int len, int level)
{
    static unsigned short frameNo = 0;
    KEFrameHead frameHead;
    frameHead.frameNo = frameNo++;
    frameHead.piecesNo = 1;
    //time set
    int ams = talk_base::Time();
    frameHead.second = ams/1000;
    frameHead.millisecond = (ams%1000)/10;

    if(level == 1){
        frameHead.frameType = video1_frame_type_;
    }else if(level == 2){
        frameHead.frameType = video2_frame_type_;
    }

    //frame type:2-CIF
    frameHead.frameLen = len;
    talk_base::Buffer frameBuf(&frameHead,sizeof(KEFrameHead));
    frameBuf.AppendData(data,len);

    if(level == 1){
        SignalVideoData1(frameBuf.data(),frameBuf.length());
    }else if(level == 2){
        SignalVideoData2(frameBuf.data(),frameBuf.length());
    }

}

void HisiMediaDevice::SendAudioFrame(const char *data, int len)
{
    KEFrameHead frameHead;
    frameHead.frameNo = 0;
    frameHead.piecesNo = 0;
    //time set
    int ams = talk_base::Time();
    frameHead.second = ams/1000;
    frameHead.millisecond = (ams%1000)/10;
    //frame type:2-CIF
    frameHead.frameType = 80;
    frameHead.frameLen = len+4;
    talk_base::Buffer frameBuf(&frameHead,sizeof(KEFrameHead));
    const char nalhead[4] = {0,0,0,1};
    frameBuf.AppendData(nalhead,4);
    frameBuf.AppendData(data,len);
    SignalAudioData(frameBuf.data(),frameBuf.length());
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
    //return video_clarity_;
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
    if(head->frameLen == len-dataPos){
        Raycomm_TalkPlay(0,const_cast<char *>(data+dataPos),head->frameLen,0,0);
    }else{
        LOG(WARNING)<<"HisiMediaDevice::OnRecvTalkData--- from "<<peer_id<<
                    " frame format error";
    }

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

    //    talk_base::Thread::SleepMs(2000);

    //    GetVideoFrameType();
    //    LOG(INFO)<<"HisiMediaDevice::SetVideoResolution ---" << command
    //            << ";vidoe type now is "<<video_frame_type_;

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
    if(resolution.compare("352x288") == 0){
        frameType = 2;
    }else if(resolution.compare("704x576") == 0){
        frameType = 0;
    }else if(resolution.compare("1280x720") == 0){
        frameType = 10;
    }else if(resolution.compare("176x144") == 0){
        frameType = 1;
    }else if(resolution.compare("704x288") == 0){
        frameType = 3;
    }else if(resolution.compare("320x240") == 0){
        frameType = 4;
    }else {
        LOG(WARNING)<<"GetVideoFrameType get not defined frame resolution "<<
                      resolution;
        frameType = 2;
    }

    if(level == 1){
        this->video1_frame_type_ = frameType;
    }else if(level == 2){
        this->video2_frame_type_ = frameType;
    }

    return frameType;
}
