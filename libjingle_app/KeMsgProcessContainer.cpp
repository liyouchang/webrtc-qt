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

int KeMsgProcessContainer::OpenTunnel(const std::string & peer_id)
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
    KeMsgProcess *process = new KeMsgProcess(peer_id,this);
    this->AddMsgProcess(process);
}

void KeMsgProcessContainer::OnTunnelClosed(PeerTerminalInterface *t, const std::string &peer_id)
{
    ASSERT(terminal_ == t);
    LOG(INFO)<<"KeMsgProcessContainer::OnTunnelClosed";

    std::vector<KeMsgProcess *>::iterator it = processes_.begin();
    for (; it != processes_.end(); ++it) {
      if ((*it)->peer_id() == peer_id) {
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
      if ((*it)->peer_id() == peer_id) {
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

int KeTunnelClient::SendCommand(const std::string &peer_id, const std::string &command)
{
    Json::Reader reader;
    Json::Value jmessage;
    if (!reader.parse(command, jmessage)) {
        LOG(WARNING) << "command format error. " << command;
        return 101;
    }
    jmessage[kTunnelTypeName] = "tunnel";

    Json::StyledWriter writer;
    std::string msg = writer.write(jmessage);

    return this->terminal_->SendByRouter(peer_id,msg);
}

int KeTunnelClient::StartPeerMedia(std::string peer_id, bool toStart)
{
    KeMessageProcessClient * process =dynamic_cast<KeMessageProcessClient *>( this->GetProcess(peer_id));
    if(process == NULL){
        LOG(WARNING) << "process not found "<<peer_id;
        return -1;
    }
    if(toStart){
        process->AskVideo(1,0,0);
    }
    else{
        process->AskVideo(1,1,1);
    }
    return 0;
}

int KeTunnelClient::DownloadRemoteFile(std::string peer_id, std::string remote_file_name)
{
    KeMessageProcessClient * process =dynamic_cast<KeMessageProcessClient *>( this->GetProcess(peer_id));
    if(process == NULL){
        LOG(WARNING) << "process not found "<<peer_id;
        return -1;
    }

    process->ReqestPlayFile(remote_file_name.c_str());

    return 0;
}



void KeTunnelClient::OnTunnelOpened(PeerTerminalInterface *t, const std::string &peer_id)
{
    ASSERT(this->terminal_ == t);
    KeMessageProcessClient * process = new KeMessageProcessClient(peer_id,this);
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
    bool ret = GetStringFromJsonObject(jmessage, kTunnelCommandName, &command);
    if(!ret){
        LOG(WARNING)<<"get command error"<<peer_id;
        return;
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

void KeTunnelClient::OnRecordFileData(const std::string &peer_id, const char *data, int len)
{
    LOG(INFO)<<"KeTunnelClient::OnRecordFileData";

}

void KeTunnelClient::OnRecordStatus(const std::string &peer_id, int status)
{
    LOG(INFO)<<"KeTunnelClient::OnRecordStatus---"<<peer_id<<" status "<<status;

}



void KeTunnelCamera::OnTunnelOpened(PeerTerminalInterface *t, const std::string &peer_id)
{
    ASSERT(terminal_ == t);
    LOG(INFO)<<__FUNCTION__<<"---------"<<peer_id;
    KeMessageProcessCamera *process = new KeMessageProcessCamera(peer_id,this);
    this->AddMsgProcess(process);

}

void KeTunnelCamera::OnRecvVideoClarity(std::string peer_id, int clarity)
{
    LOG(INFO)<<"KeTunnelCamera::OnRecvVideoClarity---" <<peer_id<<" clarity:"<<clarity ;
}

void KeTunnelCamera::OnRecvRecordQuery( std::string peer_id,  std::string condition)
{
    LOG(INFO)<<"KeTunnelCamera::OnRecvRecordQuery---" <<peer_id<<" query:"<<condition ;
}

void KeTunnelCamera::SetPtz(std::string ptz_key, int param)
{
    LOG(INFO)<<"KeTunnelCamera::SetPtz---key:" <<ptz_key<<" param:"<<param ;
}

void KeTunnelCamera::OnRecvGetWifiInfo(std::string peer_id)
{
    LOG(INFO)<<"KeTunnelCamera::OnRecvGetWifiInfo---from:" <<peer_id;
}

void KeTunnelCamera::SetWifiInfo(std::string peer_id, std::string param)
{
    LOG(INFO)<<"KeTunnelCamera::SetWifiInfo---from:" <<peer_id<<" param:"<<param ;
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
    bool ret = GetStringFromJsonObject(jmessage, kTunnelCommandName, &command);
    if(!ret){
        LOG(WARNING)<<"get command error-"<<command<<" from"<<peer_id ;
        return;
    }
    if(command.compare("video_clarity") == 0){
        int clarity;
        GetIntFromJsonObject(jmessage,"value",&clarity);
        OnRecvVideoClarity(peer_id,clarity);
    }else if(command.compare("ptz") == 0){
        std::string ptz_control;
        GetStringFromJsonObject(jmessage,"control",&ptz_control);
        int param;
        GetIntFromJsonObject(jmessage,"param",&param);
        std::string ptz_key = "ptz_";
        ptz_key += ptz_control;
        this->SetPtz(ptz_key,param);
    }else if(command.compare("query_record") == 0){
        std::string condition;
        GetStringFromJsonObject(jmessage,"result",&condition);
        OnRecvRecordQuery(peer_id,condition);
    }
    else{
        LOG(WARNING)<<"receive unexpected command from "<<peer_id;
    }

}
