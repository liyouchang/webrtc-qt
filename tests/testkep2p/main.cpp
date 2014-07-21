#include <iostream>

#include "libjingle_app/kelocalclient.h"
#include "talk/base/logging.h"
#include "libjingle_app/peerterminal.h"
#include "zmqclient/peerconnectionclientdealer.h"
using namespace std;

int main()
{

    kaerp2p::P2PConductor::AddIceServer("stun:192.168.40.179:5389","","");
    //kaerp2p::P2PConductor::AddIceServer("turn:192.168.40.179:5766","lht","123456");

    talk_base::LogMessage::ConfigureLogging("tstamp thread info debug","");
    PeerConnectionClientDealer * comm = new PeerConnectionClientDealer();
    if(!comm->Connect("tcp://192.168.40.179:5555","")){
        LOG(WARNING)<<"KePlayerPlugin::Initialize---connect error";
        return 1;
    }

    kaerp2p::KeTunnelClient * client = new kaerp2p::KeTunnelClient();
    kaerp2p::PeerTerminal * terminal = new kaerp2p::PeerTerminal();
    terminal->Initialize(comm);

    client->Init(terminal);
   // client->SearchLocalDevice();
  //  client->SearchLocalDevice();
    std::string pid = "12345678-F5C2";
    client->OpenTunnel(pid);
    //talk_base::Thread::SleepMs(5000);
//    client->StartPeerMedia(pid,3);

    talk_base::Thread::Current()->Run();

    return 0;
}

