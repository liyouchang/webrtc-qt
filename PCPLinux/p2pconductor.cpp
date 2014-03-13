#include "p2pconductor.h"

P2PConductor::P2PConductor()
{
}


void kaerp2p::P2PConductor::ConnectToPeer(std::string peer_id)
{
    if (peer_connection_.get()) {
        LOG(LS_INFO) <<"error";
        return;
    }
    if (InitializePeerConnection()) {
        peer_id_ = peer_id;
        peer_connection_->CreateOffer(this);
    } else {
        LOG(LS_INFO) <<"error";
        //main_wnd_->ShowMessageBox("Error", "Failed to initialize PeerConnection", true);
    }
}

bool kaerp2p::P2PConductor::InitializePeerConnection()
{
    PeerTunnelInterface::IceServers servers;
    PeerTunnelInterface::IceServer server;
    server.uri = GetPeerConnectionString();
    servers.push_back(server);

    stream_thread_ =  new talk_base::Thread();
    stream_thread_->Start();

    talk_base::scoped_refptr<PeerTunnel> pt (
                new talk_base::RefCountedObject<PeerTunnel>(client_thread_,stream_thread_));
    if(!pt->Initialize(servers,this)){
        return false;
    }

    peer_connection_ = PeerTunnelProxy::Create(pt->signaling_thread(), pt);

    return true;
}
