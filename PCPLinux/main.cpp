#include <iostream>

using namespace std;

#include <vector>
#include "talk/base/thread.h"
#include "ServerConductor.h"
#include "peer_connection_client.h"
#include "talk/base/logging.h"

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


int main()
{
    talk_base::LogMessage::ConfigureLogging("tstamp thread info debug",NULL);
    PeerConnectionClient client;
    talk_base::scoped_refptr<kaerp2p::ServerConductor> conductor(
                new talk_base::RefCountedObject<kaerp2p::ServerConductor>(&client));

    std::string serverIp = "218.56.11.182";
    conductor->StartLogin(serverIp,8888);
    std::cout<<"connect end"<<std::endl;

    std::vector<uint32> ids;
    ids.push_back(kaerp2p::MSG_DONE);
    ids.push_back(kaerp2p::MSG_LOGIN_FAILED);
    Loop(ids);
    return 0;
}

