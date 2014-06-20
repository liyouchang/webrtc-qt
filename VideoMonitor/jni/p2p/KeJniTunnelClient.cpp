/*
 * KeJniTunnelClient.cpp
 *
 *  Created on: 2014-4-1
 *      Author: lht
 */

#include "KeJniTunnelClient.h"
#include "talk/base/bind.h"
#include "JniUtil.h"
KeJniTunnelClient::KeJniTunnelClient() {
}

KeJniTunnelClient::~KeJniTunnelClient() {
	// TODO Auto-generated destructor stub
}
void KeJniTunnelClient::OnTunnelOpened(kaerp2p::PeerTerminalInterface * t,
                                       const std::string & peer_id){
	KeTunnelClient::OnTunnelOpened(t,peer_id);
	JniUtil::GetInstance()->JniTunnelOpened(peer_id.c_str());
}
void KeJniTunnelClient::OnRecvAudioData(const std::string& peer_id,
		const char* data, int len) {
	 JniUtil::GetInstance()->JniRecvAudioData(peer_id.c_str(),data,len);

}

void KeJniTunnelClient::OnRecvVideoData(const std::string& peer_id,
		const char* data, int len) {
	 JniUtil::GetInstance()->JniRecvVideoData(peer_id.c_str(),data,len);

}

void KeJniTunnelClient::OnRecordStatus(const std::string &peer_id, int status)
{
    JniUtil::GetInstance()->JniRecordStatus(peer_id.c_str(),status);
}

//on tunnel closed
void KeJniTunnelClient::OnTunnelClosed(kaerp2p::PeerTerminalInterface* t,
                                        const std::string& peer_id) {
	KeTunnelClient::OnTunnelClosed(t,peer_id);
    JniUtil::GetInstance()->JniTunnelMethodCallback("TunnelClosed",
                                                    peer_id.c_str());
}


KeJniLocalClient::KeJniLocalClient()
{

}

KeJniLocalClient::~KeJniLocalClient()
{

}

void KeJniLocalClient::OnTunnelOpened(kaerp2p::PeerTerminalInterface *t,
                                      const std::string &peer_id)
{
    kaerp2p::KeLocalClient::OnTunnelOpened(t,peer_id);
    JniUtil::GetInstance()->JniTunnelMethodCallback("TunnelOpened",
                                                    peer_id.c_str());

}

void KeJniLocalClient::OnTunnelClosed(kaerp2p::PeerTerminalInterface *t,
                                      const std::string &peer_id)
{
    kaerp2p::KeLocalClient::OnTunnelClosed(t,peer_id);
    JniUtil::GetInstance()->JniTunnelMethodCallback("TunnelClosed",
                                                    peer_id.c_str());
}

void KeJniLocalClient::OnSearchedDeviceInfo(const std::string &devInfo)
{
    JniUtil::GetInstance()->JniTunnelMethodCallback("SearchedDevice",
                                                    devInfo.c_str());

}

void KeJniLocalClient::OnRecvAudioData(const std::string &peer_id,
                                       const char *data, int len)
{
    JniUtil::GetInstance()->JniRecvAudioData(peer_id.c_str(),data,len);
}

void KeJniLocalClient::OnRecvVideoData(const std::string &peer_id,
                                       const char *data, int len)
{
    JniUtil::GetInstance()->JniRecvVideoData(peer_id.c_str(),data,len);
}
