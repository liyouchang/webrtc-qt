#include "KeMsgProcess.h"
#include "talk/base/stream.h"
#include "talk/base/thread.h"
#include "talk/base/stringutils.h"
#include "talk/base/json.h"

#include "peerterminal.h"
#include "KeMessage.h"
#include "p2pconductor.h"


namespace kaerp2p{

const int kHeartStop = 20; //20s without receive heart
const int kHeartDelay = 1000;  // 500 milliseconds
const int kMsgMaxLen = 512*1024;


KeMsgProcess::KeMsgProcess(std::string peer_id, KeMsgProcessContainer *container):
    peer_id_(peer_id),recvMsgMaxLen(kMsgMaxLen),heart_thread_(0),heart_count_(0),
    container_(container),heartSendDelay(kHeartDelay),heartMissStop(kHeartStop)
{
    buf_position_ = 0;
    to_read_ = 0;
}

KeMsgProcess::~KeMsgProcess()
{
}

void KeMsgProcess::OnProcessMessage(const std::string &peer_id,
                                    talk_base::Buffer &msg)
{
    //ASSERT(this->peer_id_ == peer_id);
    ASSERT(msg.length() > 0);
    this->ExtractMessage(msg);
}

void KeMsgProcess::StartHeartBeat()
{
    heart_thread_ =  talk_base::Thread::Current();
    LOG(INFO)<<"KeMsgProcess::StartHeartBeat---thread "<<heart_thread_->name();
    this->heart_count_ = 0;
    SendHeart();
    heart_thread_->PostDelayed(heartSendDelay,this,MSG_SEND_HEART);
}

void KeMsgProcess::ExtractMessage(talk_base::Buffer &allBytes)
{
    talk_base::MemoryStream buffer(allBytes.data(),allBytes.length());
    const int headLen = 10;
    talk_base::StreamResult result;
    size_t read_bytes;
    int error;
    while(true)
    {
        if (msg_received_.length() == 0)//上一个消息已经读取完成
        {
            result = buffer.Read(head_buffer_+buf_position_,
                                 headLen-buf_position_,&read_bytes,&error);
            if(result == talk_base::SR_EOS){
                break;
            }
            //消息头在最后几个字节，记录读取的字节，下次继续读取。
            if(read_bytes < headLen-buf_position_)
            {
                //LOG(INFO)<<"Continue Read head in new package\r\n ";
                buf_position_ = read_bytes;
                break;
            }
            unsigned char  protocal = head_buffer_[0];
            //int msgLen= *((int*)&headBuf[2]); //arm will failed with this
            int msgLen;
            memcpy(&msgLen,&head_buffer_[2],4);
            if (protocal != PROTOCOL_HEAD ||  msgLen > recvMsgMaxLen || msgLen < 0)
            {
                LOG(WARNING)<<"The message Protocal Head "<< read_bytes
                           <<" error, msg len "<<msgLen
                          <<" ,Clear the recv buffer!\r\n";
                msg_received_.SetLength(0);
                break;
            }
            msg_received_.SetLength(msgLen);
            buf_position_ = 0;
            memcpy(msg_received_.data(),head_buffer_,headLen);
            buf_position_ += headLen;
            to_read_ =  msgLen-headLen;
            if (to_read_ != 0)//防止 headLen 越界
            {
                result = buffer.Read(msg_received_.data()+buf_position_,
                                     to_read_,&read_bytes,&error);
                if(result == talk_base::SR_EOS){
                    break;
                }
                buf_position_ += read_bytes;
                to_read_ -= read_bytes;
            }
        }
        else//上一个消息未完成读取
        {
            result = buffer.Read(msg_received_.data()+buf_position_,
                                 to_read_,&read_bytes,&error);
            if(result == talk_base::SR_EOS){
                break;
            }
            if (read_bytes < to_read_){
                //LOG(INFO)<<"to read more and more!";
            }
            buf_position_ += read_bytes;
            to_read_ -= read_bytes;
        }
        if(to_read_ == 0 && buf_position_ == msg_received_.length())//全部读取
        {
            this->OnMessageRespond(msg_received_);
            msg_received_.SetLength(0);
            buf_position_ = 0;
        }
    }
}

void KeMsgProcess::OnMessageRespond(talk_base::Buffer &msgData)
{
    char msgType = msgData.data()[1];
    switch(msgType){
    case DevMsg_HeartBeat:
        if(heart_thread_){
            heart_thread_->Post(this,MSG_HEART_RECEIVED);
        }
        break;
    default:
        LOG(WARNING)<<"not supported message";
        break;
    }
}

void KeMsgProcess::SendHeart()
{
    talk_base::Buffer sendBuf;
    int msgLen = sizeof(KEDevMsgHead);
    sendBuf.SetLength(msgLen);
    KEDevMsgHead * pReqMsg;
    pReqMsg = (KEDevMsgHead *)sendBuf.data();
    pReqMsg->protocal = PROTOCOL_HEAD;
    pReqMsg->msgType = DevMsg_HeartBeat;
    pReqMsg->msgLength = msgLen;
    pReqMsg->videoID = 0;
    SignalNeedSendData(this->peer_id_,sendBuf.data(),sendBuf.length());
}


void KeMsgProcess::OnMessage(talk_base::Message *msg)
{
    switch(msg->message_id){
    case MSG_SEND_HEART:{
        LOG(LS_VERBOSE)<<"heart beat "<<heart_count_;
        if(++heart_count_ > heartMissStop){
            //SignalHeartStop(peer_id_);
            this->container_->OnHeartStop(peer_id_);
            break;
        }
        SendHeart();
        if(heart_thread_){
            heart_thread_->PostDelayed(heartSendDelay,this,MSG_SEND_HEART);
        }
        break;
    }
    case MSG_HEART_RECEIVED:{
        heart_count_ = 0;
        break;
    }

    }
}




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
    talk_base::CritScope cs(&crit_);
    for(int i =0 ;i< processes_.size();i++){
        delete processes_[i];
    }
}

bool KeMsgProcessContainer::Init(PeerTerminalInterface *t)
{
    //add turn server
    this->terminal_ = t;
    t->SignalTunnelOpened.connect(this,&KeMsgProcessContainer::OnTunnelOpened);
    t->SignalTunnelClosed.connect(this,&KeMsgProcessContainer::OnTunnelClosed);
    t->SignalTunnelMessage.connect(this,&KeMsgProcessContainer::OnTunnelMessage);
    t->SignalRouterMessage.connect(this,&KeMsgProcessContainer::OnRouterMessage);
    return true;
}

bool KeMsgProcessContainer::Init(kaerp2p::PeerConnectionClientInterface *client)
{
    PeerTerminal * t = new PeerTerminal(client);
    //t->Initialize(client);
    this->has_terminal = true;
    return this->Init(t);
}

bool KeMsgProcessContainer::OpenTunnel(const std::string & peer_id)
{
    ASSERT(terminal_);
    return terminal_->OpenTunnel(peer_id);
}

bool KeMsgProcessContainer::CloseTunnel(const std::string &peer_id)
{
    ASSERT(terminal_);
    return terminal_->CloseTunnel(peer_id);
}

bool KeMsgProcessContainer::IsTunnelOpened(const std::string &peer_id)
{
    return (this->GetProcess(peer_id) != NULL);
}


void KeMsgProcessContainer::OnTunnelOpened(PeerTerminalInterface *t,
                                           const std::string &peer_id)
{
    LOG_T_F(INFO)<<"tunnel opened "<<peer_id;
    ASSERT(terminal_ == t);
    KeMsgProcess *process = new KeMsgProcess(peer_id,this);
    this->AddMsgProcess(process);
}

void KeMsgProcessContainer::OnTunnelClosed(PeerTerminalInterface *t,
                                           const std::string &peer_id)
{
    ASSERT(terminal_ == t);
    talk_base::CritScope cs(&crit_);

    std::vector<KeMsgProcess *>::iterator it = processes_.begin();
    for (; it != processes_.end(); ++it) {
        if ((*it)->peer_id() == peer_id) {
            break;
        }
    }
    if (it == processes_.end()){
        LOG_T_F(WARNING)<<"peer id "<< peer_id<<" not found";
        return ;
    }
    delete (*it);
    processes_.erase(it);
    LOG_T_F(INFO)<<"delete "<<peer_id;
}

void KeMsgProcessContainer::OnTunnelMessage(const std::string &peer_id,
                                            talk_base::Buffer &msg)
{
    KeMsgProcess * process = this->GetProcess(peer_id);
    if(process == NULL){
        LOG_T_F(WARNING)<< "peer id "<< peer_id<<" not found";
        return;
    }
    process->OnProcessMessage(peer_id,msg);
}

void KeMsgProcessContainer::OnRouterMessage(const std::string &peer_id,
                                            talk_base::Buffer &msg)
{
    LOG(INFO)<<"KeMsgProcessContainer::OnRouterMessage"
            <<"---- peer_id = "<<peer_id;
}

KeMsgProcess *KeMsgProcessContainer::GetProcess(const std::string &peer_id)
{
    talk_base::CritScope cs(&crit_);
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
    LOG_T_F(INFO)<<" new process "<<process->peer_id();
    process->SignalNeedSendData.connect(
                this,&KeMsgProcessContainer::SendProcessData);
    process->SignalHeartStop.connect(this,&KeMsgProcessContainer::OnHeartStop);
    process->StartHeartBeat();
    talk_base::CritScope cs(&crit_);
    processes_.push_back(process);
}

void KeMsgProcessContainer::SendProcessData(const std::string &peer_id,
                                            const char *data, int len)
{
    if(!terminal_->SendByTunnel(peer_id,data,len)) {
        LOG(LS_VERBOSE)<<"KeMsgProcessContainer::SendSelfData---"<<
                         "Send to tunnel failed";
    }
}

void KeMsgProcessContainer::SendSelfData(const std::string &peer_id, const char *data, int len)
{
    if(!terminal_->SendByRouter(peer_id,data,len)){
        LOG(LS_VERBOSE)<<"KeMsgProcessContainer::SendSelfData---"<<
                         "Send to router failed";
    }
}

void KeMsgProcessContainer::OnHeartStop(const std::string &peer_id)
{
    LOG(WARNING)<<"heart stop close tunnel";
    this->CloseTunnel(peer_id);
}


}

