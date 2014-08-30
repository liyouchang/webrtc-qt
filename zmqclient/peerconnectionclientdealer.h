#ifndef PEERCONNECTIONCLIENTDEALER_H
#define PEERCONNECTIONCLIENTDEALER_H


#include "libjingle_app/PeerConnectionClinetInterface.h"
#include "asyndealer.h"

class PeerConnectionClientDealer :
        public kaerp2p::PeerConnectionClientInterface,
        public sigslot::has_slots<>
{
public:
    PeerConnectionClientDealer();
    virtual ~PeerConnectionClientDealer();
    //router
    virtual bool Connect(const std::string & router,const std::string & id);
    virtual void Reconnect();
    //server
    void SendEcho(const std::string & data);
    //peer
    bool SendToPeer(const std::string &peer_id, const std::string &message);
    bool IsSendingMessage();
    void OnMessageFromPeer(const std::string& peer_id, const std::string& message);
protected:
    AsynDealer * dealer_;
};



#endif // PEERCONNECTIONCLIENTDEALER_H
