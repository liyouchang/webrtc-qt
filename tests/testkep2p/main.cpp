#include <iostream>

#include "libjingle_app/kelocalclient.h"
#include "talk/base/logging.h"

using namespace std;

int main()
{

    talk_base::LogMessage::ConfigureLogging("tstamp thread info debug","");

    kaerp2p::KeLocalClient * client = new kaerp2p::KeLocalClient();
    kaerp2p::LocalTerminal * local = new kaerp2p::LocalTerminal();
    local->Initialize();
    client->Init(local);
    client->SearchLocalDevice();
  //  client->SearchLocalDevice();
    talk_base::Thread::SleepMs(1000);

 //   client->OpenTunnel("10.10.0.100:22616");
    //client->StartPeerMedia("192.168.40.152:22616");

    talk_base::Thread::Current()->Run();

    return 0;
}

