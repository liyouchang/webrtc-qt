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
	jni_thread = talk_base::Thread::Current();
}

JniPeerConnection::~JniPeerConnection() {

}

struct SendToPeerData : public talk_base::MessageData{
	std::string peer_id;
	std::string message;
	SendToPeerData(std::string pid,std::string msg):peer_id(pid),message(msg){}
};
bool JniPeerConnection::SendToPeer(const std::string& peer_id,
		const std::string& message) {
	LOGI("JniPeerConnection::SendToPeer----");


//	bool ret =  jni_thread->Invoke<bool>(
//			talk_base::Bind(&JniPeerConnection::SendToPeer_jni, this,peer_id,message));
//	bool ret =  JniUtil::GetInstance()->JniSendToPeer(peer_id.c_str(),message.c_str());

	jni_thread->Post(this,MSG_SENDTOPEER,new SendToPeerData(peer_id,message));
	//LOGI("jni invoke %d",ret);

	return true;
}

bool JniPeerConnection::SendToPeer_jni(const std::string& peer_id,
		const std::string& message) {
	LOGI("JniPeerConnection::SendToPeer_jni");

	return JniUtil::GetInstance()->JniSendToPeer(peer_id.c_str(),message.c_str());
}

void JniPeerConnection::OnMessage(talk_base::Message* msg) {
	LOGI("JniPeerConnection::OnMessage");

	if(MSG_SENDTOPEER == msg->message_id){
		SendToPeerData * pd = static_cast<SendToPeerData *>(msg->pdata);
		SendToPeer_jni(pd->peer_id,pd->message);
		delete msg->pdata;
	}
}




