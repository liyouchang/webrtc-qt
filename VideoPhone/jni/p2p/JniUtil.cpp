/*
 * JniUtil.cpp
 *
 *  Created on: 2014-4-1
 *      Author: lht
 */

#include "JniUtil.h"
#include <unistd.h>
JniUtil::JniUtil() {
	g_vm_ = NULL;
}
JniUtil::~JniUtil() {
	// TODO Auto-generated destructor stub
}

jboolean JniUtil::getObjectField(JNIEnv* env, jobject obj,
		const char* fieldName, const char* fieldDescriptor, jobject* out) {

	jclass clazz = env->GetObjectClass(obj);
	if (clazz == NULL)
		return JNI_FALSE;
	jfieldID fieldID = env->GetFieldID(clazz, fieldName, fieldDescriptor);
	env->DeleteLocalRef(clazz);
	if (fieldID == NULL)
		return JNI_FALSE;
	*out = env->GetObjectField(obj, fieldID);
	return JNI_TRUE;

}

JniUtil* JniUtil::GetInstance() {
	static JniUtil ju;
	return &ju;
}

jboolean JniUtil::getIntField(JNIEnv* env, jobject obj, const char* fieldName,
		jint* out) {
	jclass clazz = env->GetObjectClass(obj);
	if (clazz == NULL)
		return JNI_FALSE;
	jfieldID fieldID = env->GetFieldID(clazz, fieldName, "I");
	env->DeleteLocalRef(clazz);
	if (fieldID == NULL)
		return JNI_FALSE;
	*out = env->GetIntField(obj, fieldID);
	return JNI_TRUE;

}

bool JniUtil::JniSendToPeer(const char* peer_id, const char* message) {

	if(g_vm_ == 0){
		LOGE("g_vm is null");
		return false;
	}
	JNIEnv *p_env;
	bool attached = false;
	int status = g_vm_->GetEnv((void **)&p_env,JNI_VERSION_1_6);
	if(status != JNI_OK){
		//LOGI("g_vm GetEnv error %d", status);
		status = g_vm_->AttachCurrentThread(&p_env,NULL);
		if(status != JNI_OK){
			LOGE("g_vm AttachCurrentThread error %d", status);
			return false ;
		}
		attached = true;
	}

	jclass callBackCls = p_env->GetObjectClass(g_obj_);
	if (!callBackCls) {
		LOGE("Get class %s error", call_class_name_.c_str());
		return false;
	}

	jmethodID mid = p_env->GetMethodID(callBackCls, "SendToPeer",
			"(Ljava/lang/String;Ljava/lang/String;)V");
	if (!mid) {
		LOGE("get method SendToPeer error");
		return false;
	}

	jstring jni_pid = p_env->NewStringUTF(peer_id);
	jstring jni_msg = p_env->NewStringUTF(message);
	LOGI("JniUtil::JniSendToPeer----message: %s", message);


	p_env->CallVoidMethod(g_obj_, mid, jni_pid, jni_msg);

	if(attached){
		g_vm_->DetachCurrentThread();
	}
	LOGI("JniUtil::JniSendToPeer----end");

	return true;
}

bool JniUtil::JniRecvVideoData(const char* peer_id, const char* data, int len) {

	if(g_vm_ == 0){
		LOGE("g_vm is null");
		return false;
	}
	JNIEnv *p_env;
	bool attached = false;
	int status = g_vm_->GetEnv((void **)&p_env,JNI_VERSION_1_6);
	if(status != JNI_OK){
		//LOGI("g_vm GetEnv error %d", status);
		status = g_vm_->AttachCurrentThread(&p_env,NULL);
		if(status != JNI_OK){
			LOGE("g_vm AttachCurrentThread error %d", status);
			return false ;
		}
		attached = true;
	}

	jclass callBackCls = p_env->GetObjectClass(g_obj_);
	if (!callBackCls) {
		LOGE("Get class %s error", call_class_name_.c_str());
		return false;
	}

	jmethodID mid = p_env->GetMethodID(callBackCls, "RecvVideoData",
			"(Ljava/lang/String;[B)V");
	if (!mid) {
		LOGE("get method RecvVideoData error");
		return false;
	}
	jstring jni_pid = p_env->NewStringUTF(peer_id);
	jbyteArray byteArr = p_env->NewByteArray(len);
	jboolean isCopy;
	void *rd = p_env->GetPrimitiveArrayCritical((jarray) byteArr, &isCopy);
	memcpy(rd, data, len);

	p_env->CallVoidMethod(g_obj_, mid, jni_pid, byteArr);

	p_env->ReleasePrimitiveArrayCritical(byteArr, rd, JNI_ABORT);
	p_env->DeleteLocalRef(byteArr);

	if(attached){
		g_vm_->DetachCurrentThread();
	}
	return true;

}



bool JniUtil::JniRecvAudioData(const char* peer_id, const char* data, int len) {

	if(g_vm_ == 0){
		LOGE("g_vm is null");
		return false;
	}
	JNIEnv *p_env;
	bool attached = false;
	int status = g_vm_->GetEnv((void **)&p_env,JNI_VERSION_1_6);
	if(status != JNI_OK){
		//LOGI("g_vm GetEnv error %d", status);
		status = g_vm_->AttachCurrentThread(&p_env,NULL);
		if(status != JNI_OK){
			LOGE("g_vm AttachCurrentThread error %d", status);
			return false ;
		}
		attached = true;
	}

	jclass callBackCls = p_env->GetObjectClass(g_obj_);
	if (!callBackCls) {
		LOGE("Get class %s error", call_class_name_.c_str());
		return false;
	}

	jmethodID mid = p_env->GetMethodID(callBackCls, "RecvAudioData",
			"(Ljava/lang/String;[B)V");
	if (!mid) {
		LOGE("get method RecvVideoData error");
		return false;
	}
	jstring jni_pid = p_env->NewStringUTF(peer_id);
	jbyteArray byteArr = p_env->NewByteArray(len);
	jboolean isCopy;
	void *rd = p_env->GetPrimitiveArrayCritical((jarray) byteArr, &isCopy);
	memcpy(rd, data, len);

	p_env->CallVoidMethod(g_obj_, mid, jni_pid, byteArr);

	p_env->ReleasePrimitiveArrayCritical(byteArr, rd, JNI_ABORT);
	p_env->DeleteLocalRef(byteArr);

	if(attached){
		g_vm_->DetachCurrentThread();
	}

	return true;
}

bool JniUtil::getSendToPeerMethod(JNIEnv* env, jobject obj) {
	callBackCls = env->FindClass(call_class_name_.c_str());
	if (!callBackCls) {
		LOGE("Get class %s error", call_class_name_.c_str());
		return false;
	}
	SendToPeer_mid = env->GetStaticMethodID(callBackCls, "SendToPeer",
			"(Ljava/lang/String;Ljava/lang/String;)V");
	if (!SendToPeer_mid) {
		LOGE("get method SendToPeer error");
		return false;
	}
	return true;
}


