/*
 * JniUtil.h
 *
 *  Created on: 2014-4-1
 *      Author: lht
 */

#ifndef JNIUTIL_H_
#define JNIUTIL_H_
#include <jni.h>
#include <android/log.h>
#include <string>
#define LOG_TAG "JNI"
#define LOGI(...) __android_log_print(4,LOG_TAG, __VA_ARGS__);
#define LOGE(...) __android_log_print(6,LOG_TAG, __VA_ARGS__);


class JniUtil {
public:
  JniUtil();

  static JniUtil * GetInstance();
  virtual ~JniUtil();
  static jboolean getObjectField(JNIEnv *env, jobject obj, const char *fieldName, const char *fieldDescriptor, jobject *out);
  static jboolean getIntField(JNIEnv *env, jobject obj, const char *fieldName, jint *out);
  bool JniSendToPeer(const char * peer_id,const char * message);
  bool JniRecvVideoData(const char * peer_id,const char* data, int len);
  bool JniRecvAudioData(const char * peer_id,const char* data, int len);
  bool JniTunnelOpened(const char * peer_id);
  bool JniRecordStatus(const char * peer_id, int recordstatus,int playPos,int playSpeed);
  bool JniMediaStatus(const char * peer_id, int video,int audio,int talk);

  //method call back
  bool JniTunnelMethodCallback(const char * methodName, const char * strParam);
  bool JniTunnelMethodCallback(const char * methodName,const char * peer_id,const char* data, int len);
  bool getSendToPeerMethod(JNIEnv *env, jobject obj);
  JavaVM * g_vm_;
  jobject g_obj_;
  jclass callBackCls;
  jmethodID SendToPeer_mid;
  std::string call_class_name_;
protected:
};

#endif /* JNIUTIL_H_ */





