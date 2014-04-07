#include "KeVideoSimulator.h"
#include "talk/base/pathutils.h"
#include "talk/base/bind.h"
#include "talk/base/logging.h"
#include "talk/base/stream.h"
#include "../libjingle_app/KeMessage.h"
KeVideoSimulator::KeVideoSimulator()
{
    media_thread_ = new talk_base::Thread();
    media_thread_->Start();
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
    //start read file cycle
    media_thread_->Post(this,MSG_SENDFILEVIDEO);

    return true;
}

void KeVideoSimulator::SendMediaMsg(const char * data,int len)
{
    KEFrameHead * pFrame = (KEFrameHead *)data;
    int mediaFormat = pFrame->frameType & 0x7f;

    if(mediaFormat <30 ){
        this->SignalVideoData(data,len);
    }else {
        this->SignalAudioData(data,len);
    }
}

void KeVideoSimulator::OnMessage(talk_base::Message *msg)
{
    if(msg->message_id == MSG_SENDFILEVIDEO){
        static int fileBufPos = 0;
        static int lastFrameNo = 0;
        if(fileBufPos > (video_data_.length() - sizeof(KEFrameHead))){
            fileBufPos = 0;
        }
        KEFrameHead * pHead = (KEFrameHead *)(video_data_.data() + fileBufPos);
        int frameLen = pHead->frameLen + sizeof(KEFrameHead);
        if( frameLen < 0 || fileBufPos + frameLen > video_data_.length()  ){
            fileBufPos = 0;
            media_thread_->Post(this,MSG_SENDFILEVIDEO);
            return;
        }
        //send media
        this->SendMediaMsg(video_data_.data() + fileBufPos,frameLen);

        fileBufPos += frameLen;

        if(lastFrameNo == 0){
            lastFrameNo = pHead->frameNo;
        }

        if(lastFrameNo != pHead->frameNo){
            lastFrameNo = pHead->frameNo;
            media_thread_->PostDelayed(40,this ,MSG_SENDFILEVIDEO);
        }
        else{
            media_thread_->Post(this,MSG_SENDFILEVIDEO);
        }
    }
}
