#include "KeMsgProcessContainer.h"

#include "talk/base/json.h"

#include "peerterminal.h"
#include "KeMsgProcess.h"

const char kTunnelTypeName[] = "type";
const char kTunnelCommandName[] = "command";


KeMsgProcessContainer::KeMsgProcessContainer()
{
    this->terminal_ = 0;
    has_terminal = false;
}

KeMsgProcessContainer::~KeMsgProcessContainer()
{
    if(has_terminal){
        delete this->terminal_;
        this->terminal_ = 0;
    }

    for(int i =0 ;i< processes_.size();i++){
        delete processes_[i];
    }
}

bool KeMsgProcessContainer::Init(PeerTerminalInterface *t)
{
    this->terminal_ = t;
    t->SignalTunnelOpened.connect(this,&KeMsgProcessContainer::OnTunnelOpened);
    t->SignalTunnelClosed.connect(this,&KeMsgProcessContainer::OnTunnelClosed);
    t->SignalTunnelMessage.connect(this,&KeMsgProcessContainer::OnTunnelMessage);
    t->SignalRouterMessage.connect(this,&KeMsgProcessContainer::OnRouterMessage);
    return true;
}

bool KeMsgProcessContainer::Init(kaerp2p::PeerConnectionClientInterface *client)
{
    PeerTerminal * t = new PeerTerminal();
    t->Initialize(client);
    this->has_terminal = true;
    return this->Init(t);
}

int KeMsgProcessContainer::OpenTunnel(const std::string &peer_id)
{
    ASSERT(terminal_);
    int ret =  terminal_->OpenTunnel(peer_id);
    if(ret == 0){//start opened

    }
    return ret;
}

int KeMsgProcessContainer::CloseTunnel(const std::string &peer_id)
{
    ASSERT(terminal_);
    return terminal_->CloseTunnel(peer_id);
}

void KeMsgProcessContainer::OnTunnelOpened(PeerTerminalInterface *t, const std::string &peer_id)
{
    ASSERT(terminal_ == t);
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
    process->SignalHeartStop.connect(this,&KeMsgProcessContainer::OnHeartStop);
    process->StartHeartBeat();
    processes_.push_back(process);

}

void KeMsgProcessContainer::OnProcessNeedSend(const std::string &peer_id, const char *data, int len)
{
    int ret = terminal_->SendByTunnel(peer_id,data,len);
    if(ret != 0){
        LOG(LS_VERBOSE)<<"Send to tunnel failed";
    }

}

void KeMsgProcessContainer::OnHeartStop(const std::string &peer_id)
{
    LOG(WARNING)<<"heart stop close tunnel";
    this->CloseTunnel(peer_id);
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
/**
 * @brief KeTunnelClient::PeerVideoClarity
 * @param peer_id
 * @param clarity --- 1:low,2:normal,3:heigh. 1~3 is to set clarity.
 *                      101:get camera clarity
 * @return
 */
int KeTunnelClient::PeerVideoClarity(std::string peer_id, int clarity)
{
    Json::StyledWriter writer;
    Json::Value jmessage;
    jmessage[kTunnelTypeName] = "tunnel";
    jmessage[kTunnelCommandName] = "video_clarity";
    jmessage["value"] = clarity;
    std::string msg = writer.write(jmessage);
    LOG(INFO) <<"SetPeerVideoClarity msg is " << msg;
    return this->terminal_->SendByRouter(peer_id,msg);
}
/**
 * @brief KeTunnelClient::QueryRecordList
 * @param peer_id
 * @param condition : condition is a json like :
 *  {"starttime":"20140417183600","endtime":"20140418183600","offset""0,"query_num":30}
 *  the query_num's max size is 30.
 *
 * @return
 */
int KeTunnelClient::QueryRecordList(std::string peer_id, const std::string &condition)
{
    Json::StyledWriter writer;
    Json::Value jmessage;
    jmessage[kTunnelTypeName] = "tunnel";
    jmessage[kTunnelCommandName] = "query_record";
    jmessage["condition"] = condition;
    std::string msg = writer.write(jmessage);
    LOG(INFO) <<"SetPeerVideoClarity msg is " << msg;
    return this->terminal_->SendByRouter(peer_id,msg);

}

void KeTunnelClient::OnTunnelOpened(PeerTerminalInterface *t, const std::string &peer_id)
{
    ASSERT(this->terminal_ == t);
    KeMessageProcessClient * process = new KeMessageProcessClient(peer_id);
    process->SignalRecvAudioData.connect(this,&KeTunnelClient::OnRecvAudioData);
    process->SignalRecvVideoData.connect(this,&KeTunnelClient::OnRecvVideoData);
    this->AddMsgProcess(process);
}

void KeTunnelClient::OnRouterMessage(const std::string &peer_id, const std::string &msg)
{
    Json::Reader reader;
    Json::Value jmessage;
    if (!reader.parse(msg, jmessage)) {
        LOG(WARNING) << "Received unknown message. " << msg;
        return;
    }
    std::string command;
    GetStringFromJsonObject(jmessage, kTunnelCommandName, &command);
    if(command.compare("video_clarity") == 0){
        int clarity;
        GetIntFromJsonObject(jmessage,"value",&clarity);
        OnRecvVideoClarity(peer_id,clarity);
    }else{
        LOG(WARNING)<<"receive unexpected command from "<<peer_id;
    }

}

void KeTunnelClient::OnRecvAudioData(const std::string &peer_id, const char *data, int len)
{
    LOG(INFO)<<__FUNCTION__;
}

void KeTunnelClient::OnRecvVideoData(const std::string &peer_id, const char *data, int len)
{
    LOG(INFO)<<__FUNCTION__;
}

void KeTunnelClient::OnRecvVideoClarity(const std::string &peer_id, int clarity)
{
    LOG(INFO)<<"KeTunnelClient::OnRecvVideoClarity---" <<peer_id<<" clarity:"<<clarity ;

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

void KeTunnelCamera::OnRecvVideoClarity(const std::string &peer_id, int clarity)
{
    LOG(INFO)<<"KeTunnelCamera::OnRecvVideoClarity---" <<peer_id<<" clarity:"<<clarity ;

}

void KeTunnelCamera::OnRouterMessage(const std::string &peer_id, const std::string &msg)
{
    Json::Reader reader;
    Json::Value jmessage;
    if (!reader.parse(msg, jmessage)) {
        LOG(WARNING) << "Received unknown message. " << msg;
        return;
    }
    std::string command;
    GetStringFromJsonObject(jmessage, kTunnelCommandName, &command);
    if(command.compare("video_clarity") == 0){
        int clarity;
        GetIntFromJsonObject(jmessage,"value",&clarity);
        OnRecvVideoClarity(peer_id,clarity);
    }else{
        LOG(WARNING)<<"receive unexpected command from "<<peer_id;
    }

}
