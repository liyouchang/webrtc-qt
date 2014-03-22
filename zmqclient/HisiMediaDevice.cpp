#include "HisiMediaDevice.h"

HisiMediaDevice::HisiMediaDevice()
{
    process_ = new KeMessageProcessCamera();
    process_->SignalRecvAskVideoMsg.connect(this,&HisiMediaDevice::OnSendVideo);
    file_thread_ = new talk_base::Thread();
    file_thread_->Start();

}

void HisiMediaDevice::OnTunnelOpend(PeerTerminalInterface *t, const std::string &peer_id)
{
    process_->SetTerminal(peer_id,t);


}

void HisiMediaDevice::OnSendVideo()
{

}
