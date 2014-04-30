#include "HisiMediaDevice.h"

#include "talk/base/bind.h"
#include "talk/base/thread.h"
#include "talk/base/timeutils.h"
#include "talk/base/buffer.h"
#include "talk/base/logging.h"

#include "keapi/keapi.h"
#include "libjingle_app/KeMessage.h"
#include "libjingle_app/KeMsgProcess.h"

#define VIDEO1_DATA				"video1_data"
#define VIDEO1_CODEC 			"video1_codec"      	// 	0:H264, 1:MJPG
#define VIDEO1_BITRATECTRL 		"video1_control"		// 	0:CBR,  1:VBR
#define VIDEO1_BITRATE 			"video1_bitrate"		//
#define VIDEO1_QUALITY 			"video1_quality"		// 	5:普通,7:较好,9:最好
#define VIDEO1_FRAMERATE 		"video1_framerate"		//	1,5,10,15,20,25,30
#define VIDEO1_IFRAMEINTERVAL 	"video1_int"			// 	1,4,20,100,200
#define VIDEO1_DEINTERLACE 		"video1_deint"			// 	0:close,1:open反交错
#define VIDEO1_RESOLUTION 		"video1_resolution"		// 	176x144, 352x288, 320x240 640x480 1280x720

#define AUDIO1_DATA				"audio_data"

#define PTZ_MOVE_LEFT "ptz_move_left"
#define PTZ_MOVE_RIGTE "ptz_move_right"
#define PTZ_MOVE_UP "ptz_move_up"
#define PTZ_MOVE_DOWN "ptz_move_down"
#define PTZ_STOP "ptz_stop"

const int kVideoSampleRate = 40;//40 ms per frame
const int kAudioSampleRate = 20;//20 ms

struct MediaControlData : public talk_base::MessageData {
    int video;//0 to request start, 1 to request stop
    int audio;
    MediaControlData(int v, int a) : video(v), audio(a) { }
};


HisiMediaDevice::HisiMediaDevice():
    media_thread_(0),video_handle_(0),audio_handle_(0)
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

    LOG(INFO)<<"vidoe frame type "<<GetVideoFrameType();


    media_thread_ = new talk_base::Thread();
    media_thread_->Start();

    //start get media
    media_thread_->Post(this,MSG_MEDIA_CONTROL,new MediaControlData(0,0));


    KeTunnelCamera::Init(client);
}



void HisiMediaDevice::SendVideoFrame(const char *data, int len)
{
    static unsigned short frameNo = 0;
    KEFrameHead frameHead;
    frameHead.frameNo = frameNo++;
    frameHead.piecesNo = 1;
    //time set
    int ams = talk_base::Time();
    frameHead.second = ams/1000;
    frameHead.millisecond = (ams%1000)/10;
    //frame type:2-CIF
    frameHead.frameType = video_frame_type_;
    frameHead.frameLen = len;
    talk_base::Buffer frameBuf(&frameHead,sizeof(KEFrameHead));
    frameBuf.AppendData(data,len);
    SignalVideoData(frameBuf.data(),frameBuf.length());
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
    }
}

void HisiMediaDevice::SetPtz(std::string ptz_key, int param)
{
    LOG(INFO)<<"HisiMediaDevice::SetPtz---key:" <<ptz_key<<" param:"<<param ;
    Raycomm_SetPtz(ptz_key.c_str(),param,0);
}

void HisiMediaDevice::OnMessage(talk_base::Message *msg)
{
    switch (msg->message_id) {
    case MSG_MEDIA_CONTROL:{
        MediaControlData* mcd = static_cast<MediaControlData*>(msg->pdata);
        if(mcd->video == 0 && video_handle_ == 0){
            video_handle_ =  Raycomm_ConnectMedia(VIDEO1_DATA,0);
            LOG(INFO)<<"HisiMediaDevice start video" <<video_handle_;
            media_thread_->Post(this,HisiMediaDevice::MSG_SEND_VIDEO);
        }
        else if(mcd->video != 0 && video_handle_ != 0){
            Raycomm_DisConnectMedia(video_handle_);
            LOG(INFO)<<"HisiMediaDevice stop video";
            video_handle_ = 0;
        }

        if(mcd->audio == 0 && audio_handle_ == 0 ){
            audio_handle_ =  Raycomm_ConnectMedia(AUDIO1_DATA,0);
            LOG(INFO)<<"HisiMediaDevice start audio" <<audio_handle_;
            media_thread_->Post(this,HisiMediaDevice::MSG_SEND_AUDIO);
        }
        else if(mcd->audio != 0 && audio_handle_ != 0){

            Raycomm_DisConnectMedia(audio_handle_);
            LOG(INFO)<<"HisiMediaDevice stop audio" ;
            audio_handle_ = 0;
        }

        delete msg->pdata;
        break;
    }
    case HisiMediaDevice::MSG_SEND_VIDEO:{
        if(video_handle_ == 0) break;
        unsigned int timespan;
        int media_len = Raycomm_GetMediaData(video_handle_,media_buffer_,MEDIA_BUFFER_LENGTH,&timespan);
        if(media_len > 0){
            if(media_len == MEDIA_BUFFER_LENGTH){
                LOG(LS_ERROR)<<"Raycomm_GetMediaData---no enough buffer for this large frame";
            }
            this->SendVideoFrame(media_buffer_,media_len);
            //get next frame
            media_thread_->Post(this,HisiMediaDevice::MSG_SEND_VIDEO);
        }
        else{
            //wait and get
            media_thread_->PostDelayed(kVideoSampleRate,this,
                                       HisiMediaDevice::MSG_SEND_VIDEO);
        }
        break;
    }
    case HisiMediaDevice::MSG_SEND_AUDIO:{
        if(audio_handle_ == 0) break;
        unsigned int timespan;
        int media_len = Raycomm_GetMediaData(audio_handle_,media_buffer_,MEDIA_BUFFER_LENGTH,&timespan);
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

    talk_base::Thread::SleepMs(2000);

    GetVideoFrameType();
    LOG(INFO)<<"HisiMediaDevice::SetVideoResolution ---" << command
            << ";vidoe type now is "<<video_frame_type_;

}

int HisiMediaDevice::GetVideoFrameType()
{
    char buf[1024];
    memset(buf,0,1024);
    Raycomm_GetParam(VIDEO1_RESOLUTION,buf,0);
    std::string resolution(buf);
    if(resolution.compare("352x288") == 0){
        video_frame_type_ = 2;
    }else if(resolution.compare("704x576") == 0){
        video_frame_type_ = 0;
    }else if(resolution.compare("1280x720") == 0){
        video_frame_type_ = 10;
    }else if(resolution.compare("176x144") == 0){
        video_frame_type_ = 1;
    }else if(resolution.compare("704x288") == 0){
        video_frame_type_ = 3;
    }else if(resolution.compare("320x240") == 0){
        video_frame_type_ = 4;
    }else {
        LOG(WARNING)<<"GetVideoFrameType get not defined frame resolution "<<resolution;
        video_frame_type_ = 2;
    }
    return video_frame_type_;
}
