/*
 * JniUtil.cpp
 *
 *  Created on: 2014-4-1
 *      Author: lht
 */

#include "JniUtil.h"
#include <unistd.h>
JniUtil::JniUtil() {
	g_env_ = NULL;
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
	LOGI("JniUtil::JniSendToPeer 1");
	JNIEnv *p_env;


	int status = g_vm_->GetEnv((void **)&p_env,JNI_VERSION_1_6);
	if(status != JNI_OK){
		LOGI("g_vm GetEnv error %d", status);
		status = g_vm_->AttachCurrentThread(&p_env,NULL);
		if(status != JNI_OK){
			LOGE("g_vm AttachCurrentThread error %d", status);
			return false ;
		}
	}
	jclass callBackCls = p_env->FindClass(call_class_name_.c_str());
	if (!callBackCls) {
		LOGE("Get class %s error", call_class_name_.c_str());
		return false;
	}
	LOGI("JniUtil::JniSendToPeer 2");

	jmethodID mid = p_env->GetStaticMethodID(callBackCls, "SendToPeer",
			"(Ljava/lang/String;Ljava/lang/String;)V");
	if (!mid) {
		LOGE("get method SendToPeer error");
		return false;
	}
	jstring jni_pid = p_env->NewStringUTF(peer_id);
	jstring jni_msg = p_env->NewStringUTF(message);

	LOGI("message: %s", message);
	const char * msg1 = p_env->GetStringUTFChars(jni_msg,NULL);
	LOGI("msg1: %s", msg1);

	p_env->CallStaticVoidMethod(callBackCls, mid, jni_pid, jni_msg);
	return true;
}

bool JniUtil::JniRecvVideoData(const char* peer_id, const char* data, int len) {
	jclass callBackCls = g_env_->FindClass(call_class_name_.c_str());
	if (!callBackCls) {
		LOGE("Get class %s error", call_class_name_.c_str());
		return false;
	}
	jmethodID mid = g_env_->GetMethodID(callBackCls, "RecvVideoData",
			"(Ljava/lang/String;[B)V");
	if (!mid) {
		LOGE("get method RecvVideoData error");
		return false;
	}
	jstring jni_pid = g_env_->NewStringUTF(peer_id);

	jbyteArray byteArr = g_env_->NewByteArray(len);
	jboolean isCopy;
	void *rd = g_env_->GetPrimitiveArrayCritical((jarray) byteArr, &isCopy);
	memcpy(rd, data, len);

	g_env_->CallVoidMethod(callBackCls, mid, jni_pid, byteArr);

	g_env_->ReleasePrimitiveArrayCritical(byteArr, rd, JNI_ABORT);
	g_env_->DeleteLocalRef(byteArr);
	return true;

}

bool JniUtil::JniRecvAudioData(const char* peer_id, const char* data, int len) {
	jclass callBackCls = g_env_->FindClass(call_class_name_.c_str());
	if (!callBackCls) {
		LOGE("Get class %s error", call_class_name_.c_str());
		return false;
	}
	jmethodID mid = g_env_->GetMethodID(callBackCls, "RecvAudioData",
			"(Ljava/lang/String;[B)V");
	if (!mid) {
		LOGE("get method RecvAudioData error");
		return false;
	}
	jstring jni_pid = g_env_->NewStringUTF(peer_id);

	jbyteArray byteArr = g_env_->NewByteArray(len);
	jboolean isCopy;
	void *rd = g_env_->GetPrimitiveArrayCritical((jarray) byteArr, &isCopy);
	memcpy(rd, data, len);

	g_env_->CallVoidMethod(callBackCls, mid, jni_pid, byteArr);

	g_env_->ReleasePrimitiveArrayCritical(byteArr, rd, JNI_ABORT);
	g_env_->DeleteLocalRef(byteArr);
	return true;
}

