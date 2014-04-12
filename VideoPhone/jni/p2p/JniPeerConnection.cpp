/*
 * JniPeerConnection.cpp
 *
 *  Created on: 2014-4-1
 *      Author: lht
 */

#include "JniPeerConnection.h"
#include "talk/base/bind.h"
#include "JniUtil.h"
JniPeerConnection::JniPeerConnection() {
	//jni_thread = talk_base::Thread::Current();
}

JniPeerConnection::~JniPeerConnection() {

}

//struct SendToPeerData : public talk_base::MessageData{
//	std::string peer_id;
//	std::string message;
//	SendToPeerData(std::string pid,std::string msg):peer_id(pid),message(msg){}
//};
bool JniPeerConnection::SendToPeer(const std::string& peer_id,
		const std::string& message) {
	bool ret =  JniUtil::GetInstance()->JniSendToPeer(peer_id.c_str(),message.c_str());
	return ret;
}





