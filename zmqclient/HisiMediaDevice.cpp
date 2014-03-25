#include "HisiMediaDevice.h"

#include "keapi/keapi.h"
#include "talk/base/bind.h"
#include "talk/base/thread.h"
#include "talk/base/timeutils.h"
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

void HisiMediaDevice::OnTunnelOpened(PeerTerminalInterface *t, const std::string &peer_id)
{
    process_.reset(new KeMessageProcessCamera());
    process_->SignalRecvAskMediaMsg.connect(this,&HisiMediaDevice::OnMediaRequest);
    process_->SetTerminal(peer_id,t);

}

void HisiMediaDevice::OnTunnelClosed(PeerTerminalInterface *t, const std::string &peer_id)
{
    OnMediaRequest(1,1);
    process_.reset();

}

void HisiMediaDevice::OnMediaRequest(int video,int audio)
{
    media_thread_->Invoke<int>(
        talk_base::Bind(&HisiMediaDevice::MediaControl_m,this,video,audio));
}

int HisiMediaDevice::MediaControl_m(int video, int audio)
{
    if(video == 0 && video_handle_ == 0){
        video_handle_ =  Raycomm_ConnectMedia(VIDEO1_DATA,0);
        LOG(INFO)<<"start video";
        media_thread_->Post(this,HisiMediaDevice::MSG_SEND_VIDEO);
    }
    else{
        Raycomm_DisConnectMedia(video_handle_);
        LOG(INFO)<<"stop video";
        video_handle_ = 0;
    }
}

void HisiMediaDevice::SendFrame(int type, const char *data, int len)
{
    static short frameNo = 0;
    KEFrameHead frameHead;
    frameHead.frameNo = frameNo++;
    frameHead.piecesNo = 1;
    int ams = talk_base::Time();
    frameHead.second = ams/1000;
    frameHead.millisecond = (ams%1000)/10;
    frameHead.frameLen = len;
    frameHead.frameType = 2;
    talk_base::Buffer frameBuf(&frameHead,sizeof(KEFrameHead));
    frameBuf.AppendData(data,len);
    process_->SendMediaMsg(type,frameBuf.data(),frameBuf.length());

}

void HisiMediaDevice::OnMessage(talk_base::Message *msg)
{
    switch (msg->message_id) {
    case HisiMediaDevice::MSG_SEND_VIDEO:{
        if(video_handle_ == 0){
            break;
        }
        unsigned int timespan;
        int media_len;
        media_len = Raycomm_GetMediaData(video_handle_,media_buffer_,MEDIA_BUFFER_LENGTH,&timespan);
        if(media_len > 0 && process_){
            LOG(LS_VERBOSE)<<"read media len = "<<media_len;
            //LOG(INFO)<<"read media len = "<<media_len<< " time "<<timespan;
            this->SendFrame(KEMSG_TYPE_VIDEOSTREAM,media_buffer_,media_len);
            media_thread_->Post(this,HisiMediaDevice::MSG_SEND_VIDEO);
        }
        else{
            media_thread_->PostDelayed(40,this,HisiMediaDevice::MSG_SEND_VIDEO);
        }
        break;
    }
    default:
            break;

    }
}
