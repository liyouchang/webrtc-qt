#include <jni.h>

#include <string>
#include <iostream>
#include "JniPeerConnection.h"
#include "KeJniTunnelClient.h"
#include "JniUtil.h"
#include<pthread.h>

#include "talk/base/logging.h"

KeJniTunnelClient * client = NULL;
JniPeerConnection * jniPeer = NULL;

jint naInitialize(JNIEnv *env, jobject thiz, jstring cbClass) {
	LOGI("1. naInitialize()");

	env->GetJavaVM(&JniUtil::GetInstance()->g_vm_);
	JniUtil::GetInstance()->g_obj_ = env->NewGlobalRef(thiz);

	jniPeer = new JniPeerConnection();
	client = new KeJniTunnelClient();
	client->Initialize(jniPeer);

	return 0;
}
jint naTerminate(JNIEnv *env, jobject thiz) {
	LOGI("2. naTerminate()");
	if (jniPeer) {
		delete jniPeer;
		jniPeer = NULL;
	}
	if (client) {
		delete client;
		client = NULL;
	}

	return 0;
}

jint naOpenTunnel(JNIEnv *env, jobject thiz, jstring peer_id) {
	LOGI("3. naOpenTunnel()");
	if (client == NULL) {
		return -1;
	}

	const char * pid = env->GetStringUTFChars(peer_id, NULL);
	return client->ConnectToPeer(pid);
}
jint naMessageFromPeer(JNIEnv *env, jobject thiz, jstring peer_id, jstring message){
		if (jniPeer == NULL) {
			return -1;
		}
		const char * pid = env->GetStringUTFChars(peer_id, NULL);
		const char * pMsg = env->GetStringUTFChars(message,NULL);
		jniPeer->OnMessageFromPeer(pid,pMsg);
		return 0;

}
jint naCloseTunnel(JNIEnv *env, jobject thiz, jstring peer_id) {
	LOGI("4. naCloseTunnel()");
	if (client == NULL) {
		return -1;
	}
	const char * pid = env->GetStringUTFChars(peer_id, NULL);
	return client->CloseTunnel(pid);
}

jint naAskMediaData(JNIEnv *env, jobject thiz, jstring peer_id) {
	LOGI("5. naAskMediaData()");
	if (client == NULL) {
		return -1;
	}
	const char * pid = env->GetStringUTFChars(peer_id, NULL);
	return client->AskPeerVideo(pid);
}

#ifndef NELEM
#define NELEM(x) ((int)(sizeof(x)/sizeof((x)[0])))
#endif

jint JNI_OnLoad(JavaVM * pVm, void * reserved) {
    talk_base::LogMessage::ConfigureLogging("tstamp thread info debug",NULL);

	JNIEnv * env;
	if (pVm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
		return -1;
	}

	JNINativeMethod nm[] = {
			{ "naInitialize", "(Ljava/lang/String;)I", (void*) naInitialize },
			{ "naTerminate", "()I", (void*) naTerminate },
			{ "naOpenTunnel", "(Ljava/lang/String;)I", (void*) naOpenTunnel },
			{ "naCloseTunnel", "(Ljava/lang/String;)I", (void*) naCloseTunnel },
			{ "naAskMediaData", "(Ljava/lang/String;)I", (void*) naAskMediaData },
			{ "naMessageFromPeer", "(Ljava/lang/String;Ljava/lang/String;)I", (void*) naMessageFromPeer }
	};

	jclass cls = env->FindClass("com/video/play/TunnelCommunication");
	env->RegisterNatives(cls, nm, NELEM(nm));

	JniUtil::GetInstance()->g_vm_ = pVm;

	return JNI_VERSION_1_6;
}
