#include <iostream>

#include "libjingle_app/kelocalclient.h"
using namespace std;

int main()
{
    kaerp2p::KeLocalClient * client = new kaerp2p::KeLocalClient();
    kaerp2p::LocalTerminal * local = new kaerp2p::LocalTerminal();
    local->Initialize();
    client->Init(local);
    client->SearchLocalDevice();
  //  client->SearchLocalDevice();

    client->OpenTunnel("192.168.40.152:22616");
    talk_base::Thread::SleepMs(1000);
    client->StartPeerMedia("192.168.40.152:22616");

    talk_base::Thread::Current()->Run();

    return 0;
}

