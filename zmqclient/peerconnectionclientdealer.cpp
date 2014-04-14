#include "peerconnectionclientdealer.h"

#include "talk/base/logging.h"
#include "asyndealer.h"

// This is our magical hangup signal.
const char kByeMessage[] = "BYE";

//const std::string routerURL = "tcp://192.168.0.182:5555";

PeerConnectionClientDealer::PeerConnectionClientDealer()
{
}

int PeerConnectionClientDealer::Connect(const std::string &router, const std::string &id)
{
    dealer_.reset(new AsynDealer());
    dealer_->SignalReadData.connect(this,&PeerConnectionClientDealer::OnMessageFromPeer);
    //dealer_->SignalSent.connect(this,&PeerConnectionClientDealer::OnMessageSent);
    LOG(INFO)<<"connect to "<<router<<"  with id "<<id;
    int ret = dealer_->initialize(id,router);
    this->my_id_ = dealer_->id();
    return ret;
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


bool PeerConnectionClientDealer::IsSendingMessage()
{
    return false;
}

void PeerConnectionClientDealer::OnMessageFromPeer(const std::string &peer_id, const std::string &message)
{
    SignalMessageFromPeer(peer_id,message);
}

//void PeerConnectionClientDealer::OnMessageSent()
//{
//    this->SignalMessageSent(0);
//}

