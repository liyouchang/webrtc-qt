#include "KeVideoSimulator.h"
#include "talk/base/pathutils.h"
#include "KeMessage.h"
#include "talk/base/bind.h"
KeVideoSimulator::KeVideoSimulator()
{
    media_thread_ = new talk_base::Thread();
    media_thread_->Start();
}

KeVideoSimulator::~KeVideoSimulator()
{
    delete media_thread_;
}

void KeVideoSimulator::SetTerminal(PeerTerminalInterface *t)
{
    this->terminal_ = t;
    t->SignalTunnelOpened.connect(this,&KeVideoSimulator::OnTunnelOpened);
    t->SignalTunnelClosed.connect(this,&KeVideoSimulator::OnTunnelClosed);
    t->SignalTunnelMessage.connect(this,&KeVideoSimulator::OnTunnelMessage);


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



void KeVideoSimulator::OnTunnelOpened(PeerTerminalInterface *t, const std::string &peer_id)
{
    ASSERT(terminal_ == t);
    LOG(INFO)<<__FUNCTION__;

    KeMessageProcessCamera *process = new KeMessageProcessCamera(peer_id);
    //process->SetTerminal(peer_id,t);
    process->SignalRecvAskMediaMsg.connect(this,&KeVideoSimulator::OnProcessMediaRequest);
    processes_.push_back(process);
}

void KeVideoSimulator::OnTunnelClosed(PeerTerminalInterface *t, const std::string &peer_id)
{
    ASSERT(terminal_ == t);
    LOG(INFO)<<__FUNCTION__;

    std::vector<KeMessageProcessCamera *>::iterator it = processes_.begin();
    for (; it != processes_.end(); ++it) {
      if ((*it)->GetPeerID() == peer_id) {
        break;
      }
    }
    if (it == processes_.end()){
        LOG(WARNING)<< "peer id "<< peer_id<<" not found";
      return ;
    }

    delete (*it);
    processes_.erase(it);
}

void KeVideoSimulator::OnTunnelMessage(const std::string &peer_id, talk_base::Buffer &msg)
{
    KeMessageProcessCamera * process = this->GetProcess(peer_id);
    if(process == NULL){
        LOG(WARNING)<< "peer id "<< peer_id<<" not found";
        return;
    }
    process->OnTunnelMessage(peer_id,msg);
}

void KeVideoSimulator::OnRouterMessage(const std::string &peer_id, const std::string &msg)
{
    LOG(INFO)<<__FUNCTION__<<"---- peer_id = "<<peer_id<<" msg = "<<msg;
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

void KeVideoSimulator::OnProcessMediaRequest(KeMessageProcessCamera * process,int video,int audio)
{
    if(video == 0 ){
        this->SignalVideoData.connect(process , &KeMessageProcessCamera::OnVideoData);
    }else{
        this->SignalVideoData.disconnect(process);
    }

    if(audio == 0 ){
        this->SignalAudioData.connect(process,&KeMessageProcessCamera::OnAudioData);
    }else{
        this->SignalAudioData.disconnect(process);
    }

}

KeMessageProcessCamera *KeVideoSimulator::GetProcess(const std::string &peer_id)
{
    std::vector<KeMessageProcessCamera *>::iterator it = processes_.begin();
    for (; it != processes_.end(); ++it) {
      if ((*it)->GetPeerID() == peer_id) {
        break;
      }
    }
    if (it == processes_.end())
      return NULL;
    return *it;

}

void KeVideoSimulator::OnProcessNeedSend(const std::string &peer_id, const char *data, int len)
{
    terminal_->SendByTunnel(peer_id,data,len);
}

void KeVideoSimulator::OnMessage(talk_base::Message *msg)
{
    if(msg->message_id == MSG_SENDFILEVIDEO){
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

        if(lastFrameNo != pHead->frameNo){
            lastFrameNo = pHead->frameNo;
            media_thread_->PostDelayed(40,this ,MSG_SENDFILEVIDEO);
        }
        else{
            media_thread_->Post(this,MSG_SENDFILEVIDEO);
        }
    }
}
