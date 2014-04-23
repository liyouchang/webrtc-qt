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
	//jni_thread = talk_base::Thread::Current();
}

KeJniTunnelClient::~KeJniTunnelClient() {
	// TODO Auto-generated destructor stub
}
void KeJniTunnelClient::OnTunnelOpened(PeerTerminalInterface * t,const std::string & peer_id){
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



