#include "peerconnectionclientdealer.h"



// This is our magical hangup signal.
const char kByeMessage[] = "BYE";

const std::string routerURL = "tcp://192.168.0.182:5555";

PeerConnectionClientDealer::PeerConnectionClientDealer()
{
}

int PeerConnectionClientDealer::Connect(const std::string &router, const std::string &id)
{
    dealer_.reset(new AsynDealer());
    dealer_->SignalReadData.connect(this,&PeerConnectionClientDealer::OnMessageFromPeer);
    dealer_->SignalSent.connect(this,&PeerConnectionClientDealer::OnMessageSent);
    int ret = dealer_->initialize(id,router);
    this->my_id_ = dealer_->id();
    return ret;
}

void PeerConnectionClientDealer::StartLogin(const std::string &serverURL, const std::string &client_name)
{
    //TODO:add login message to server

}

void PeerConnectionClientDealer::SendEcho(const std::string &data)
{
    dealer_->AsynSend(dealer_->id(),data);
}

bool PeerConnectionClientDealer::SendToPeer(const std::string &peer_id, const std::string &message)
{
    ASSERT(dealer_ != NULL);
    dealer_->AsynSend(peer_id,message);
    return true;
}

bool PeerConnectionClientDealer::SendHangUp(const std::string &peer_id)
{
    ASSERT(dealer_ != NULL);

    std::string message = kByeMessage;
    dealer_->AsynSend(peer_id,message);
    return  true;
}

bool PeerConnectionClientDealer::IsSendingMessage()
{
    return false;
}

void PeerConnectionClientDealer::OnMessageFromPeer(const std::string &peer_id, const std::string &message)
{
//TODO:message dispath
    if(message.length() == (sizeof(kByeMessage) - 1) &&
            message.compare(kByeMessage) == 0){
        SignalPeerDisConnected(peer_id);
    }else{
        //callback_->OnMessageFromPeer(peer_id,message);
        SignalMessageFromPeer(peer_id,message);
    }
}

void PeerConnectionClientDealer::OnMessageSent()
{
    this->SignalMessageSent(0);
}

