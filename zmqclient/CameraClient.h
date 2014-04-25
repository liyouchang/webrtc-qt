#ifndef CAMERACLIENT_H
#define CAMERACLIENT_H

#include "peerconnectionclientdealer.h"
#include "talk/base/messagehandler.h"

namespace talk_base {
class Thread;
}  // namespace talk_base

class CameraClient: public PeerConnectionClientDealer,
        public talk_base::MessageHandler
{
public:
    enum
    {
        MSG_LOGIN_TIMEOUT
    };
    explicit CameraClient(std::string mac);
    virtual void Login();

    // MessageHandler interface
public:
    void OnMessage(talk_base::Message *msg);
private:
    talk_base::Thread * comm_thread_;
    std::string mac_;

    // PeerConnectionClientInterface interface
public:
    void OnMessageFromPeer(const std::string &peer_id, const std::string &message);
};

#endif // CAMERACLIENT_H
