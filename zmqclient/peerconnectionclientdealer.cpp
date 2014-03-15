#include "peerconnectionclientdealer.h"



// This is our magical hangup signal.
const char kByeMessage[] = "BYE";

const std::string routerURL = "tcp://192.168.0.182:5555";

PeerConnectionClientDealer::PeerConnectionClientDealer()
{
}

int PeerConnectionClientDealer::Connect(const std::string &router, const std::string &id)
{
    dealer = new AsynDealer();
    dealer->SignalReadData.connect(this,&PeerConnectionClientDealer::OnMessageFromPeer);
    dealer->SignalSent.connect(this,&PeerConnectionClientDealer::OnMessageSent);
    int ret = dealer->initialize(id,router);
    return ret;
}

void PeerConnectionClientDealer::StartLogin(const std::string &serverURL, const std::string &client_name)
{
    //TODO:add login message to server

}

bool PeerConnectionClientDealer::SendToPeer(const std::string &peer_id, const std::string &message)
{
    ASSERT(dealer != NULL);
    dealer->AsynSend(peer_id,message);
    return true;
}

bool PeerConnectionClientDealer::SendHangUp(const std::string &peer_id)
{
    ASSERT(dealer != NULL);

    std::string message = kByeMessage;
    dealer->AsynSend(peer_id,message);
    return  true;
}

bool PeerConnectionClientDealer::IsSendingMessage()
{
    return false;
}

void PeerConnectionClientDealer::OnMessageFromPeer(const std::string &peer_id, const std::string &message)
{
//TODO:message dispath
}

void PeerConnectionClientDealer::OnMessageSent()
{
    this->SignalMessageSent(0);
}

