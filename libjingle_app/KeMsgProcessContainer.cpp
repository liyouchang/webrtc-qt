#include "KeMsgProcessContainer.h"
#include "peerterminal.h"
#include "KeMsgProcess.h"


KeMsgProcessContainer::KeMsgProcessContainer()
{
    this->terminal_ = 0;
    has_terminal = false;
}

KeMsgProcessContainer::~KeMsgProcessContainer()
{
    if(has_terminal){
        delete this->terminal_;
    }

    for(int i =0 ;i< processes_.size();i++){
        delete processes_[i];
    }
}

int KeMsgProcessContainer::Initialize(PeerTerminalInterface *t)
{
    this->terminal_ = t;
    t->SignalTunnelOpened.connect(this,&KeMsgProcessContainer::OnTunnelOpened);
    t->SignalTunnelClosed.connect(this,&KeMsgProcessContainer::OnTunnelClosed);
    t->SignalTunnelMessage.connect(this,&KeMsgProcessContainer::OnTunnelMessage);
    t->SignalRouterMessage.connect(this,&KeMsgProcessContainer::OnRouterMessage);
    return 0;
}

int KeMsgProcessContainer::Initialize(kaerp2p::PeerConnectionClientInterface *client)
{
    PeerTerminal * t = new PeerTerminal();
    t->Initialize(client);
    this->has_terminal = true;
    return this->Initialize(t);
}

int KeMsgProcessContainer::OpenTunnel(const std::string &peer_id)
{
    return terminal_->OpenTunnel(peer_id);
}

int KeMsgProcessContainer::CloseTunnel(const std::string &peer_id)
{
    return terminal_->CloseTunnel(peer_id);
}

void KeMsgProcessContainer::OnTunnelOpened(PeerTerminalInterface *t, const std::string &peer_id)
{
    KeMsgProcess *process = new KeMsgProcess(peer_id);
    AddMsgProcess(process);
}

void KeMsgProcessContainer::OnTunnelClosed(PeerTerminalInterface *t, const std::string &peer_id)
{
    ASSERT(terminal_ == t);
    LOG(INFO)<<"KeMsgProcessContainer::OnTunnelClosed";

    std::vector<KeMsgProcess *>::iterator it = processes_.begin();
    for (; it != processes_.end(); ++it) {
      if ((*it)->GetPeerID() == peer_id) {
        break;
      }
    }
    if (it == processes_.end()){
        LOG(WARNING)<< "peer id "<< peer_id<<" not found";
      return ;
    }

    LOG(INFO)<< "delete and erase peer "<< peer_id;
    delete (*it);
    processes_.erase(it);

}

void KeMsgProcessContainer::OnTunnelMessage(const std::string &peer_id, talk_base::Buffer &msg)
{
    KeMsgProcess * process = this->GetProcess(peer_id);
    if(process == NULL){
        LOG(WARNING)<< "peer id "<< peer_id<<" not found";
        return;
    }
    process->OnTunnelMessage(peer_id,msg);

}

void KeMsgProcessContainer::OnRouterMessage(const std::string &peer_id, const std::string &msg)
{
    LOG(INFO)<<__FUNCTION__<<"---- peer_id = "<<peer_id<<" msg = "<<msg;

}

KeMsgProcess *KeMsgProcessContainer::GetProcess(const std::string &peer_id)
{
    std::vector<KeMsgProcess *>::iterator it = processes_.begin();
    for (; it != processes_.end(); ++it) {
      if ((*it)->GetPeerID() == peer_id) {
        break;
      }
    }
    if (it == processes_.end())
      return NULL;
    return *it;

}

void KeMsgProcessContainer::AddMsgProcess(KeMsgProcess *process)
{
    process->SignalNeedSendData.connect(this,&KeMsgProcessContainer::OnProcessNeedSend);
    processes_.push_back(process);

}

void KeMsgProcessContainer::OnProcessNeedSend(const std::string &peer_id, const char *data, int len)
{
    int ret = terminal_->SendByTunnel(peer_id,data,len);
    if(ret != 0){
        LOG(WARNING)<<"Send to tunnel failed";
    }

}


KeTunnelClient::KeTunnelClient()
{

}

int KeTunnelClient::AskPeerVideo(std::string peer_id)
{
    KeMessageProcessClient * process =dynamic_cast<KeMessageProcessClient *>( this->GetProcess(peer_id));
    if(process == NULL){
        LOG(WARNING) << "process not found "<<peer_id;
        return -1;
    }
    process->AskVideo();
    return 0;
}

void KeTunnelClient::OnTunnelOpened(PeerTerminalInterface *t, const std::string &peer_id)
{
    ASSERT(this->terminal_ == t);
    KeMessageProcessClient * process = new KeMessageProcessClient(peer_id);
    process->SignalRecvAudioData.connect(this,&KeTunnelClient::OnRecvAudioData);
    process->SignalRecvVideoData.connect(this,&KeTunnelClient::OnRecvVideoData);
    this->AddMsgProcess(process);

}

void KeTunnelClient::OnRecvAudioData(const std::string &peer_id, const char *data, int len)
{
    LOG(INFO)<<__FUNCTION__;
}

void KeTunnelClient::OnRecvVideoData(const std::string &peer_id, const char *data, int len)
{
    LOG(INFO)<<__FUNCTION__;
}


void KeTunnelCamera::OnTunnelOpened(PeerTerminalInterface *t, const std::string &peer_id)
{
    ASSERT(terminal_ == t);
    LOG(INFO)<<__FUNCTION__<<"---------"<<peer_id;
    KeMessageProcessCamera *process = new KeMessageProcessCamera(peer_id);
    process->SignalRecvAskMediaMsg.connect(this,&KeTunnelCamera::OnProcessMediaRequest);
    this->AddMsgProcess(process);

}

void KeTunnelCamera::OnProcessMediaRequest(KeMessageProcessCamera *process, int video, int audio)
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
