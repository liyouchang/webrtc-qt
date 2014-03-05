#include "p2pthread.h"
#include <iostream>
#include <vector>
#include "talk/base/thread.h"
#include "ServerConductor.h"
#include "peer_connection_client.h"
#include "talk/base/logging.h"


P2PThread::P2PThread(QObject *parent) :
    QThread(parent)
{
}


uint32 Loop(const std::vector<uint32>& ids) {
    talk_base::Message msg;
    while (talk_base::Thread::Current()->Get(&msg)) {
        if (msg.phandler == NULL) {
            if (std::find(ids.begin(), ids.end(), msg.message_id) != ids.end()){
                return msg.message_id;
            }
            std::cout << "orphaned message: " << msg.message_id;
            continue;
        }
        talk_base::Thread::Current()->Dispatch(&msg);
    }
    return 0;
}

void P2PThread::run()
{
    talk_base::LogMessage::ConfigureLogging("tstamp thread info debug",NULL);
    PeerConnectionClient client;
    talk_base::scoped_refptr<kaerp2p::ServerConductor> conductor(
                new talk_base::RefCountedObject<kaerp2p::ServerConductor>(&client));

    std::string serverIp = "192.168.40.195";
    conductor->StartLogin(serverIp,8888);
    std::cout<<"connect end"<<std::endl;

    std::vector<uint32> ids;
    ids.push_back(kaerp2p::MSG_DONE);
    ids.push_back(kaerp2p::MSG_LOGIN_FAILED);
    Loop(ids);
}
