#include "KeVideoSimulator.h"
#include "talk/base/pathutils.h"
#include "KeMessage.h"
#include "talk/base/bind.h"
KeVideoSimulator::KeVideoSimulator()
{
    media_thread_ = new talk_base::Thread();
    media_thread_->Start();
    startSend = false;
}

KeVideoSimulator::~KeVideoSimulator()
{
    delete media_thread_;
}

bool KeVideoSimulator::ReadVideoData(std::string file_name)
{
    talk_base::Pathname path;
    bool result = talk_base::Filesystem::GetAppPathname(&path);
    if(!result){
        return false;
    }
    path.AppendPathname(file_name);

    LOG(INFO)<<"file name is "<<path.pathname();

    talk_base::FileStream * stream = talk_base::Filesystem::OpenFile(path,"r");
    if(stream == NULL){
        LOG(WARNING) << "open file error";
        return false;
    }
    size_t fileSize;
    result = stream->GetSize(&fileSize);
    if(!result){
        return false;
    }

    video_data_.SetLength(fileSize);
    size_t readData;
    int error;
    talk_base::StreamResult sr = stream->Read(video_data_.data(),fileSize,&readData,&error);
    if(sr != talk_base::SR_SUCCESS){
        return false;
    }
    return true;
}



void KeVideoSimulator::OnTunnelOpened(PeerTerminalInterface *t, const std::string &peer_id)
{
    LOG(INFO)<<__FUNCTION__;
    process_.reset(new KeMessageProcessCamera());
    process_->SetTerminal(peer_id,t);
    process_->SignalRecvAskMediaMsg.connect(this,&KeVideoSimulator::OnMediaRequest);
}

void KeVideoSimulator::OnTunnelClosed(PeerTerminalInterface *t, const std::string &peer_id)
{
    LOG(INFO)<<__FUNCTION__;
    startSend = false;
    process_.reset();
}

void KeVideoSimulator::SendMediaMsg(const char * data,int len)
{

    KEFrameHead * pFrame = (KEFrameHead *)data;
    int mediaFormat = pFrame->frameType & 0x7f;
    int msgType = mediaFormat <30 ?KEMSG_TYPE_VIDEOSTREAM:KEMSG_TYPE_AUDIOSTREAM;

    if(process_){
        process_->SendMediaMsg(msgType,data,len);
    }
}

void KeVideoSimulator::OnMediaRequest(int video,int audio)
{

    media_thread_->Invoke<int>(
                talk_base::Bind(&KeVideoSimulator::MediaRequest_m,this,video,audio));

}

int KeVideoSimulator::MediaRequest_m(int video, int audio)
{
    if(!startSend && video==0){
        LOG(INFO)<<"Start send video";

        media_thread_->Post(this,MSG_SENDFILEVIDEO);
        startSend = true;
    }

    if(video == 1){
        startSend = false;
    }

}

void KeVideoSimulator::OnMessage(talk_base::Message *msg)
{
    if(msg->message_id == MSG_SENDFILEVIDEO){

        if(!startSend){
            return;
        }
        static int fileBufPos = 0;
        static int lastFrameNo = 0;
        if(fileBufPos > video_data_.length()){
            fileBufPos = 0;
        }
        KEFrameHead * pHead = (KEFrameHead *)(video_data_.data() + fileBufPos);
        int frameLen = pHead->frameLen + sizeof(KEFrameHead);
        //send media
        this->SendMediaMsg(video_data_.data() + fileBufPos,frameLen);

        fileBufPos += frameLen;

        if(lastFrameNo == 0){
            lastFrameNo = pHead->frameNo;
        }

        if(startSend){
            if(lastFrameNo != pHead->frameNo){
                //LOG(INFO)<<"read next frame";
                lastFrameNo = pHead->frameNo;
                media_thread_->PostDelayed(40,this ,MSG_SENDFILEVIDEO);
            }
            else{
                //LOG(INFO)<<"read same frame";
                media_thread_->Post(this,MSG_SENDFILEVIDEO);
            }
        }
    }
}
