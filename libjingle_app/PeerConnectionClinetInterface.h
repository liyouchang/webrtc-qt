#ifndef PEERCONNECTIONCLINETINTERFACE_H
#define PEERCONNECTIONCLINETINTERFACE_H

#include "talk/base/sigslot.h"
#include <string>
namespace kaerp2p {



//struct PeerConnectionClientObserver :public sigslot::has_slots<>{
//    virtual void OnSignedIn() {}  // Called when we're logged on.
//    virtual void OnDisconnected() {}
//    virtual void OnPeerConnected(const std::string& id, const std::string& name){}
//    virtual void OnPeerDisconnected(const std::string& peer_id){}
//    virtual void OnMessageFromPeer(const std::string& peer_id, const std::string& message){}
//    virtual void OnMessageSent(int err){}
//    virtual void OnServerConnectionFailure(){}
//protected:
//    virtual ~PeerConnectionClientObserver() {}
//};

class PeerConnectionClientInterface
{
public:
   virtual int Connect(const std::string & router,const std::string & id) = 0;

    //send message to peer, it should be thread safe
    virtual bool SendToPeer(const std::string &peer_id, const std::string& message) =0;
    virtual bool IsSendingMessage() = 0;
//    virtual void RegisterObserver(PeerConnectionClientObserver * callback){
//        this->SignalSignedIn.connect(
//                    callback,&PeerConnectionClientObserver::OnSignedIn);
//        this->SignalDisconnected.connect(
//                    callback,&PeerConnectionClientObserver::OnDisconnected);
//        this->SignalPeerConnected.connect(
//                    callback,&PeerConnectionClientObserver::OnPeerConnected);
//        this->SignalPeerDisConnected.connect(
//                    callback,&PeerConnectionClientObserver::OnPeerDisconnected);
//        this->SignalMessageFromPeer.connect(
//                    callback,&PeerConnectionClientObserver::OnMessageFromPeer);
//        this->SignalMessageSent.connect(
//                    callback,&PeerConnectionClientObserver::OnMessageSent);
//        this->SignalServerConnectionFailure.connect(
//                    callback,&PeerConnectionClientObserver::OnServerConnectionFailure);
//        callback_ = callback;
//    }
    virtual  void OnMessageFromPeer(const std::string& peer_id, const std::string& message) = 0;

    virtual const std::string & id() const {return my_id_;}
    sigslot::signal0<> SignalSignedIn;
    sigslot::signal0<> SignalDisconnected;
    sigslot::signal2<const std::string& , const std::string&> SignalPeerConnected;
    sigslot::signal1<const std::string&> SignalPeerDisConnected;
    sigslot::signal2<const std::string &,const std::string &> SignalMessageFromPeer;
    sigslot::signal1<int> SignalMessageSent;
    sigslot::signal0<> SignalServerConnectionFailure;

protected:
    std::string my_id_;

    //PeerConnectionClientObserver* callback_;
    virtual ~PeerConnectionClientInterface() {}

};




}

#endif // PEERCONNECTIONCLINETINTERFACE_H
