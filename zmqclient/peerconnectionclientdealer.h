#ifndef PEERCONNECTIONCLIENTDEALER_H
#define PEERCONNECTIONCLIENTDEALER_H


#include "PeerConnectionClinetInterface.h"
#include "asyndealer.h"

class PeerConnectionClientDealer :
        public kaerp2p::PeerConnectionClientInterface,
        public sigslot::has_slots<>
{
public:
    PeerConnectionClientDealer();

private:
    AsynDealer * dealer;
    std::string peer_id_;
    // PeerConnectionClientInterface interface
public:
    //router
    int Connect(const std::string & router,const std::string & id);
    //server
    void StartLogin(const std::string& serverURL,const std::string& client_name);

    //peer
    bool SendToPeer(const std::string &peer_id, const std::string &message);
    bool SendHangUp(const std::string &peer_id);
    bool IsSendingMessage();
protected:
    //message is use client to get and send
    void OnMessageFromPeer(const std::string& peer_id, const std::string& message);
    void OnMessageSent();
};



#endif // PEERCONNECTIONCLIENTDEALER_H
