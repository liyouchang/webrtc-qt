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
#define AUDIO1_DATA				"audio_data"

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
    media_thread_ = new talk_base::Thread();
    media_thread_->Start();

    int ret = Raycomm_InitParam();
    LOG(INFO)<<"Raycomm_InitParam : "<<ret;

    ret = Raycomm_MediaDataInit();
    LOG(INFO)<<"Raycomm_MediaDataInit : "<<ret;

    KeTunnelCamera::Init(client);
}


void HisiMediaDevice::OnTunnelClosed(PeerTerminalInterface *t, const std::string &peer_id)
{
    //media_thread_->Send(this,HisiMediaDevice::MSG_STOP_VIDEO);
    KeTunnelCamera::OnTunnelClosed(t,peer_id);


    int video_count,audio_count;
    CountVideoAndAudio(video_count,audio_count);
    int video_start = 0;
    int audio_start = 0;
    if(video_count == 0) video_start = 1;
    if(audio_count == 0) audio_start = 1;
    media_thread_->Post(this,MSG_MEDIA_CONTROL,new MediaControlData(video_start,audio_start));


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
    frameHead.frameType = 2;
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
    frameHead.frameLen = len;
    talk_base::Buffer frameBuf(&frameHead,sizeof(KEFrameHead));
    frameBuf.AppendData(data,len);
    SignalAudioData(frameBuf.data(),frameBuf.length());

}

void HisiMediaDevice::CountVideoAndAudio(int &video_num, int &audio_num)
{
    int cv = 0;
    int ca = 0;
    std::vector<KeMsgProcess *>::iterator it = processes_.begin();
    for (; it != processes_.end(); ++it) {
        KeMessageProcessCamera * pc = static_cast<KeMessageProcessCamera *>(*it);
        if(pc->start_video_){
            cv++;
        }
        if(pc->start_audio_){
            ca++;
        }
    }
    video_num = cv;
    audio_num = ca;
}



void HisiMediaDevice::OnMessage(talk_base::Message *msg)
{
    switch (msg->message_id) {
    case MSG_MEDIA_CONTROL:{
        MediaControlData* mcd = static_cast<MediaControlData*>(msg->pdata);
        if(mcd->video == 0 && video_handle_ == 0){
            video_handle_ =  Raycomm_ConnectMedia(VIDEO1_DATA,0);
            LOG(INFO)<<"start video" <<video_handle_;
            media_thread_->Post(this,HisiMediaDevice::MSG_SEND_VIDEO);
        }
        else if(mcd->video != 0 && video_handle_ != 0){
            Raycomm_DisConnectMedia(video_handle_);
            LOG(INFO)<<"stop video";
            video_handle_ = 0;
        }

        if(mcd->audio == 0 && audio_handle_ == 0 ){
            audio_handle_ =  Raycomm_ConnectMedia(AUDIO1_DATA,0);
            LOG(INFO)<<"start audio" <<audio_handle_;
            media_thread_->Post(this,HisiMediaDevice::MSG_SEND_AUDIO);
        }
        else if(mcd->audio != 0 && audio_handle_ != 0){

            Raycomm_DisConnectMedia(audio_handle_);
            LOG(INFO)<<"stop audio" ;
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
        }
        media_thread_->PostDelayed(kAudioSampleRate,this,
                                   HisiMediaDevice::MSG_SEND_AUDIO);
        break;
    }
    default:
        break;

    }
}

void HisiMediaDevice::OnProcessMediaRequest(KeMessageProcessCamera *process, int video, int audio)
{

    int video_count,audio_count;
    CountVideoAndAudio(video_count,audio_count);

    int video_stop = 0;
    int audio_stop = 0;
    if(video_count == 0) video_stop = 1;
    if(audio_count == 0) audio_stop = 1;
    media_thread_->Post(this,MSG_MEDIA_CONTROL,new MediaControlData(video_stop,audio_stop));


    KeTunnelCamera::OnProcessMediaRequest(process,video,audio);
}
