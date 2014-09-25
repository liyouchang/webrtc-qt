#include <iostream>

#include "libjingle_app/kelocalclient.h"
#include "talk/base/logging.h"
#include "libjingle_app/peerterminal.h"
#include "zmqclient/peerconnectionclientdealer.h"
using namespace std;

int main()
{
    talk_base::LogMessage::ConfigureLogging("tstamp thread info debug","");

    kaerp2p::P2PConductor::AddIceServer("stun:218.56.11.181:5389","","");
    //kaerp2p::P2PConductor::AddIceServer("turn:192.168.40.179:5766","lht","123456");


    PeerConnectionClientDealer * comm = new PeerConnectionClientDealer();
    if(!comm->Connect("tcp://218.56.11.181:5555","")){
        LOG(WARNING)<<"KePlayerPlugin::Initialize---connect error";
        return 1;
    }
    kaerp2p::PeerTerminal * terminal = new kaerp2p::PeerTerminal(comm);

//    kaerp2p::LocalUdpTerminal * terminal = new kaerp2p::LocalUdpTerminal();
//    terminal->Initialize("");

    kaerp2p::KeTunnelClient * client = new kaerp2p::KeTunnelClient();
    client->Init(terminal);

   // client->SearchLocalDevice();
  //  client->SearchLocalDevice();
    std::string pid = "008387914747-C00F";
   // std::string pid = "10.10.0.151:22555";
    client->OpenTunnel(pid);

    talk_base::Thread::Current()->ProcessMessages(5000);
    client->StartPeerMedia(pid,1);
    talk_base::Thread::Current()->ProcessMessages(1000);

    client->StartPeerVideoCut(pid,"test.mp4");
//    client->StartPeerMedia(pid,1);
    talk_base::Thread::Current()->ProcessMessages(20000);

    client->StopPeerVideoCut(pid);

    talk_base::Thread::Current()->ProcessMessages(1000);

//    talk_base::Thread::Current()->Run();

    delete client;
    delete terminal;
    delete comm;
    return 0;
}

