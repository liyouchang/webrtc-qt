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

void KeJniTunnelClient::OnRecvAudioData(const std::string& peer_id,
		const char* data, int len) {
	//jni_thread->Invoke<void>(
	//		talk_base::Bind(&KeJniTunnelClient::RecvAudioData_jni, this,peer_id,data,len));

}

void KeJniTunnelClient::OnRecvVideoData(const std::string& peer_id,
		const char* data, int len) {
	//jni_thread->Invoke<void>(
	//		talk_base::Bind(&KeJniTunnelClient::RecvVideoData_jni, this,peer_id,data,len));

}

void KeJniTunnelClient::RecvAudioData_jni(const std::string& peer_id,
		const char* data, int len) {
	 JniUtil::GetInstance()->JniRecvAudioData(peer_id.c_str(),data,len);

}

void KeJniTunnelClient::RecvVideoData_jni(const std::string& peer_id,
		const char* data, int len) {
	 JniUtil::GetInstance()->JniRecvVideoData(peer_id.c_str(),data,len);

}



