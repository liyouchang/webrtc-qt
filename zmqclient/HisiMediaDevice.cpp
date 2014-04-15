#include "HisiMediaDevice.h"

#include "keapi/keapi.h"
#include "talk/base/bind.h"
#include "talk/base/thread.h"
#include "talk/base/timeutils.h"
#include "talk/base/buffer.h"
#include "libjingle_app/KeMessage.h"

#define VIDEO1_DATA				"video1_data"

struct MediaRequest{
    int video;
    int audio;
};

typedef talk_base::ScopedMessageData<MediaRequest> MediaRequestMessage;

HisiMediaDevice::HisiMediaDevice()
{

    media_thread_ = new talk_base::Thread();
    media_thread_->Start();
    int ret = Raycomm_InitParam();
    LOG(INFO)<<"Raycomm_InitParam : "<<ret;

    ret = Raycomm_MediaDataInit();
    LOG(INFO)<<"Raycomm_MediaDataInit : "<<ret;

    video_handle_ = 0;
}

HisiMediaDevice::~HisiMediaDevice()
{
    Raycomm_MediaDataUnInit();
    delete media_thread_;
}

//void HisiMediaDevice::OnTunnelOpened(PeerTerminalInterface *t, const std::string &peer_id)
//{
//    process_.reset(new KeMessageProcessCamera());
//    process_->SignalRecvAskMediaMsg.connect(this,&HisiMediaDevice::OnMediaRequest);
//    process_->SetTerminal(peer_id,t);

//}

void HisiMediaDevice::OnTunnelClosed(PeerTerminalInterface *t, const std::string &peer_id)
{
    OnMediaRequest(1,1);
    KeTunnelCamera::OnTunnelClosed(t,peer_id);
}

void HisiMediaDevice::OnMediaRequest(int video,int audio)
{
}

int HisiMediaDevice::MediaControl_m(int video, int audio)
{
}

void HisiMediaDevice::SendVideoFrame(const char *data, int len)
{
    static short frameNo = 0;
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

void HisiMediaDevice::OnMessage(talk_base::Message *msg)
{
    switch (msg->message_id) {
    case HisiMediaDevice::MSG_START_VIDEO:{
        video_handle_ =  Raycomm_ConnectMedia(VIDEO1_DATA,0);
        LOG(INFO)<<"start video";
        media_thread_->Post(this,HisiMediaDevice::MSG_SEND_VIDEO);
        break;
    }
    case HisiMediaDevice::MSG_STOP_VIDEO:{
        Raycomm_DisConnectMedia(video_handle_);
        LOG(INFO)<<"stop video";
        video_handle_ = 0;
        break;
    }
    case HisiMediaDevice::MSG_SEND_VIDEO:{
        if(video_handle_ == 0){
            break;
        }
        unsigned int timespan;
        int media_len;
        media_len = Raycomm_GetMediaData(video_handle_,media_buffer_,MEDIA_BUFFER_LENGTH,&timespan);
        if(media_len > 0){
            LOG(LS_VERBOSE)<<"read media len = "<<media_len;
            this->SendVideoFrame(media_buffer_,media_len);
            //get next frame
            media_thread_->Post(this,HisiMediaDevice::MSG_SEND_VIDEO);
        }
        else{
            //wait and get
            media_thread_->PostDelayed(40,this,HisiMediaDevice::MSG_SEND_VIDEO);
        }
        break;
    }
    default:
            break;

    }
}

void HisiMediaDevice::OnProcessMediaRequest(KeMessageProcessCamera *process, int video, int audio)
{
    if(video == 0 && video_handle_ == 0){
        media_thread_->Post(this,HisiMediaDevice::MSG_START_VIDEO);
    }
    else if(video != 0 && video_handle_ != 0 ){
        media_thread_->Post(this,HisiMediaDevice::MSG_STOP_VIDEO);
    }

    KeTunnelCamera::OnProcessMediaRequest(process,video,audio);
}
