#ifndef PEERCONNECTIONCLIENTDEALER_H
#define PEERCONNECTIONCLIENTDEALER_H


#include "libjingle_app/PeerConnectionClinetInterface.h"
#include "talk/base/scoped_ptr.h"
#include "asyndealer.h"

class PeerConnectionClientDealer :
        public kaerp2p::PeerConnectionClientInterface,
        public sigslot::has_slots<>
{
public:
    PeerConnectionClientDealer();
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
    talk_base::scoped_ptr<AsynDealer> dealer_;
};



#endif // PEERCONNECTIONCLIENTDEALER_H
