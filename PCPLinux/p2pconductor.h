#ifndef P2PCONDUCTOR_H
#define P2PCONDUCTOR_H

#include "talk/app/kaerp2p/KaerSession.h"
#include "peer_connection_client.h"
#include <iostream>
#include "streamprocess.h"
#include <list>
#include "peertunnel.h"


namespace kaerp2p {

class P2PClientInterface
{
public:
    virtual bool SendToPeer(std::string peer_id, const std::string& message) =0;
    virtual bool SendHangUp(std::string peer_id) =0;
    virtual void OnReadPeerData() = 0;
};


class P2PConductor:
        public webrtc::CreateSessionDescriptionObserver,
        public PeerTunnelObserver,
        public talk_base::MessageHandler
{
public:
    P2PConductor();

    virtual void ConnectToPeer(std::string peer_id);
    virtual void DisconnectFromCurrentPeer();
    virtual void WriteToPeer();

protected:
    bool InitializePeerConnection();
    void DeletePeerConnection();

    P2PClientInterface client_;
};

}
#endif // P2PCONDUCTOR_H
