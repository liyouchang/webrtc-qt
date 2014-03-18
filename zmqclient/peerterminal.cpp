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
    conductor_->SignalStreamOpened.connect(this,&PeerTerminal::OnTunnelOpened);
    this->tunnelStream_ = NULL;
    return ret;
}

void PeerTerminal::ConnectToPeer(const std::string & peer_id)
{
    conductor_->ConnectToPeer(peer_id);


}

void PeerTerminal::CloseTunnel()
{
    conductor_->DisconnectFromCurrentPeer();
    this->tunnelStream_ = NULL;
}

int PeerTerminal::SendByRouter(const std::string &peer_id, const std::string &data)
{
    client_->SendToPeer(peer_id,data);
    return 0;
}

int PeerTerminal::SendByTunnel(const std::string &data)
{
    ASSERT(tunnelStream_);
    tunnelStream_->WriteStream(data.c_str(),data.length());
    return 0;
}

int PeerTerminal::SendByTunnel(const char *data, size_t len)
{
    ASSERT(tunnelStream_);
    tunnelStream_->WriteStream(data,len);
    return 0;

}

void PeerTerminal::OnTunnelOpened(kaerp2p::StreamProcess *tunnel)
{
    LOG(INFO)<< __FUNCTION__;
    this->tunnelStream_ = tunnel;
    tunnelStream_->SignalReadData.connect(this,&PeerTerminal::OnTunnelReadData);
}

void PeerTerminal::OnTunnelReadData(kaerp2p::StreamProcess *tunnel, size_t len)
{
    //LOG(INFO)<< __FUNCTION__ << " read " << len;
    char  * buffer = new char[len];
    size_t readLen;
    tunnel->ReadStream(buffer,len,&readLen);
    //LOG(INFO) <<"read len "<<readLen << "  buffer "<< buffer;
    delete buffer;
}
