#include <jni.h>

#include <string>
#include <iostream>
#include "JniPeerConnection.h"
#include "KeJniTunnelClient.h"
#include "JniUtil.h"

KeJniTunnelClient * client = NULL;
JniPeerConnection * jniPeer = NULL;

jint naInitialize(JNIEnv *env,jobject thiz,jstring cbClass){

	JniUtil::GetInstance()->g_env_ = env;
	JniUtil::GetInstance()->call_class_name_ = env->GetStringUTFChars(cbClass,NULL);

	jniPeer = new JniPeerConnection();
	client = new KeJniTunnelClient();
	client->Initialize(jniPeer);

	return 0;
}
jint naTerminate(JNIEnv *env,jobject thiz){
	if(jniPeer){
		delete jniPeer;
		jniPeer = NULL;
	}
	if(client){
		delete client;
		client = NULL;
	}

	return 0;
}

jint naOpenTunnel(JNIEnv *env,jobject thiz,jstring peer_id){
	if(client == NULL){
		return -1;
	}
	const char * pid = env->GetStringUTFChars(peer_id,NULL);
	return client->ConnectToPeer(pid);
}

jint naCloseTunnel(JNIEnv *env,jobject thiz,jstring peer_id){
	if(client == NULL){
		return -1;
	}
	const char * pid = env->GetStringUTFChars(peer_id,NULL);
	return client->CloseTunnel(pid);
}

jint naAskMediaData(JNIEnv *env,jobject thiz,jstring peer_id){
		if(client == NULL){
			return -1;
		}
		const char * pid = env->GetStringUTFChars(peer_id,NULL);
		return client->AskPeerVideo(pid);
}




#ifndef NELEM
#define NELEM(x) ((int)(sizeof(x)/sizeof((x)[0])))
#endif

jint JNI_OnLoad(JavaVM * pVm ,void * reserved){
	JNIEnv * env;
	if(pVm->GetEnv((void **)&env,JNI_VERSION_1_6) != JNI_OK ){
		return -1;
	}

	JNINativeMethod nm[] = {

	};


	jclass cls = env->FindClass("com/h264/VView");
	env->RegisterNatives(cls,nm,NELEM(nm));

	JniUtil::GetInstance()->g_vm_ = pVm;
	return JNI_VERSION_1_6;
}
