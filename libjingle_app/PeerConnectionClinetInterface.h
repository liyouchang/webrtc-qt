#ifndef PEERCONNECTIONCLINETINTERFACE_H
#define PEERCONNECTIONCLINETINTERFACE_H

#include "talk/base/sigslot.h"
#include <string>
namespace kaerp2p {

class PeerConnectionClientInterface
{
public:
   //virtual int Connect(const std::string & router,const std::string & id) = 0;
    //send message to peer, it should be thread safe
    virtual bool SendToPeer(const std::string &peer_id, const std::string& message) =0;
    virtual bool IsSendingMessage(){return false;}
    virtual  void OnMessageFromPeer(const std::string& peer_id, const std::string& message){
        SignalMessageFromPeer(peer_id,message);
    }
    sigslot::signal2<const std::string &,const std::string &> SignalMessageFromPeer;
    sigslot::signal0<> SignalDestroyed;

protected:
    virtual ~PeerConnectionClientInterface() {}
};




}

#endif // PEERCONNECTIONCLINETINTERFACE_H
