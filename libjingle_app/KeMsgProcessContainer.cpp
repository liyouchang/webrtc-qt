#include "KeMsgProcessContainer.h"

KeMsgProcessContainer::KeMsgProcessContainer()
{
    this->terminal_ = 0;

}

void KeMsgProcessContainer::Initialize(PeerTerminalInterface *t)
{
    this->terminal_ = t;
    t->SignalTunnelOpened.connect(this,&KeMsgProcessContainer::OnTunnelOpened);
    t->SignalTunnelClosed.connect(this,&KeMsgProcessContainer::OnTunnelClosed);
    t->SignalTunnelMessage.connect(this,&KeMsgProcessContainer::OnTunnelMessage);
    t->SignalRouterMessage.connect(this,&KeMsgProcessContainer::OnRouterMessage);

}

void KeMsgProcessContainer::OnTunnelOpened(PeerTerminalInterface *t, const std::string &peer_id)
{
    KeMsgProcess *process = new KeMsgProcess(peer_id);
    AddMsgProcess(process);
}

void KeMsgProcessContainer::OnTunnelClosed(PeerTerminalInterface *t, const std::string &peer_id)
{
    ASSERT(terminal_ == t);
    LOG(INFO)<<__FUNCTION__;

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
