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

    explicit CameraClient(std::string mac);
    virtual void Login();
    virtual void SendAlarm(int alarmType, const std::string &alarmInfo,
                           const std::string &picture);
    virtual bool Connect(const std::string &router, const std::string &id);
    virtual void Reconnect();

    enum
    {
        MSG_LOGIN_HEART,
        MSG_RECEIVE_HEART,
        MSG_RECONNECT
    };
    virtual void OnMessage(talk_base::Message *msg);
    virtual void OnMessageFromPeer(const std::string &peer_id, const std::string &message);
private:
    talk_base::Thread * comm_thread_;
    std::string mac_;
    std::string messageServer;
    std::string alarmServer;
    int heartCount;

};

#endif // CAMERACLIENT_H
