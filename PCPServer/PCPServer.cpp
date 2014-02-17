#include <QCoreApplication>

#include "talk/p2p/client/basicportallocator.h"

#include "talk/app/kaerp2p/kaer_session_client.h"
//#include "talk/base/win32socketinit.h"
//#include "talk/base/win32socketserver.h"
#include "talk/base/thread.h"

#include "ServerConductor.h"
#include <iostream>
#include <vector>

// Runs the current thread until a message with the given ID is seen.
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


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
//    talk_base::EnsureWinsockInit();
//    talk_base::Win32Thread w32_thread;
//    talk_base::ThreadManager::Instance()->SetCurrentThread(&w32_thread);

    PeerConnectionClient client;
    talk_base::scoped_refptr<kaerp2p::ServerConductor> conductor(
                new talk_base::RefCountedObject<kaerp2p::ServerConductor>(&client));

    std::string serverIp = "218.56.11.182";
    conductor->StartLogin(serverIp,8888);
    std::cout<<"connect end"<<std::endl;



    return a.exec();
}
