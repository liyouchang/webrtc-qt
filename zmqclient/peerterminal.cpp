#include "peerterminal.h"

PeerTerminal::PeerTerminal()
{
}

int PeerTerminal::Initialize(const std::string &router)
{
    client_.reset(new PeerConnectionClientDealer());
    int ret = client_->Connect(router,"");

    client_->SendEcho("hello me");
    //a new conductor is created at here
    conductor_ = new talk_base::RefCountedObject<kaerp2p::P2PConductor>(client_.get());
    return ret;
}

void PeerTerminal::ConnectToPeer(const std::string & peer_id)
{
    conductor_->ConnectToPeer(peer_id);


}

void PeerTerminal::CloseTunnel()
{
    conductor_->DisconnectFromCurrentPeer();
}

int PeerTerminal::SendByRouter(const std::string &peer_id, const std::string &data)
{
    client_->SendToPeer(peer_id,data);
}
