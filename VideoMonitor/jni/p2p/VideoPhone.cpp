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
KeJniLocalClient * localClient = NULL;

jint naInitialize(JNIEnv *env, jobject thiz, jstring cbClass) {
    LOGI("1. naInitialize()");

    env->GetJavaVM(&JniUtil::GetInstance()->g_vm_);
    JniUtil::GetInstance()->g_obj_ = env->NewGlobalRef(thiz);

    jniPeer = new JniPeerConnection();
    client = new KeJniTunnelClient();
    client->Init(jniPeer);

    kaerp2p::LocalTerminal * lt = new kaerp2p::LocalTerminal();
    lt->Initialize();
    localClient = new KeJniLocalClient();
    localClient->Init(lt);

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
    if(localClient){
        delete localClient;
        localClient = NULL;
    }
    return 0;
}

jint naOpenTunnel(JNIEnv *env, jobject thiz, jstring peer_id) {
    LOGI("3. naOpenTunnel()");
    if (client == NULL) {
        return -1;
    }
    int ret = 0;
    const char * pid = env->GetStringUTFChars(peer_id, NULL);
    if(!client->OpenTunnel(pid)){
        ret = -2;
    }
    env->ReleaseStringUTFChars(peer_id,pid);
    return ret;
}

jint naMessageFromPeer(JNIEnv *env, jobject thiz,
                       jstring peer_id, jstring message){
    if (jniPeer == NULL) {
        return -1;
    }
    const char * pid = env->GetStringUTFChars(peer_id, NULL);
    const char * pMsg = env->GetStringUTFChars(message,NULL);
    jniPeer->OnMessageFromPeer(pid,pMsg);
    env->ReleaseStringUTFChars(peer_id,pid);
    env->ReleaseStringUTFChars(message,pMsg);
    return 0;

}

jint naCloseTunnel(JNIEnv *env, jobject thiz, jstring peer_id) {
    LOGI("4. naCloseTunnel()");
    if (client == NULL) {
        return -1;
    }
    int ret = 0;
    const char * pid = env->GetStringUTFChars(peer_id, NULL);
    if(!client->CloseTunnel(pid)){
        ret = -2;
    }
    env->ReleaseStringUTFChars(peer_id,pid);
    return ret;
}

jint naAskMediaData(JNIEnv *env, jobject thiz, jstring peer_id) {
    LOGI("5. naAskMediaData()");
    if (client == NULL) {
        return -1;
    }
    const char * pid = env->GetStringUTFChars(peer_id, NULL);

    client->StartPeerMedia(pid,2);

    env->ReleaseStringUTFChars(peer_id,pid);
    return 0;

}


jint naSendTalkData(JNIEnv *env, jobject thiz,
                    jbyteArray talkBytes,jint dataLen) {
    LOGI("5. naSendTalkData()");
    if (client == NULL){
        return -1;
    }
    jbyte * talkData = env->GetByteArrayElements(talkBytes, 0);
    //jsize len = env->GetArrayLength(talkBytes);

    client->SendTalkData((char *)talkData,dataLen);

    env->ReleaseByteArrayElements(talkBytes,talkData,0);
    return 0;
}

jint naStartPeerVideoCut(JNIEnv *env, jobject thiz,
                         jstring peer_id,jstring filename){
    if (client == NULL){
        return -1;
    }
    const char *pid = env->GetStringUTFChars(peer_id, NULL);
    const char *file = env->GetStringUTFChars(filename,NULL);

    client->StartPeerVideoCut(pid,file);

    env->ReleaseStringUTFChars(peer_id,pid);
    env->ReleaseStringUTFChars(filename,file);
    return 0;

}

jint naStopPeerVideoCut(JNIEnv *env, jobject thiz,jstring peer_id){
    if (client == NULL){
        return -1;
    }
    const char *pid = env->GetStringUTFChars(peer_id, NULL);

    client->StopPeerVideoCut(pid);

    env->ReleaseStringUTFChars(peer_id,pid);
    return 0;
}

jint naDownloadRemoteFile(JNIEnv *env, jobject thiz,
                          jstring peerId,jstring remoteFileName,
                          jstring toSaveFileName,jint toPlaySize){
    if (client == NULL){
        return -1;
    }
    const char *pid = env->GetStringUTFChars(peerId, NULL);
    const char *remoteFile = env->GetStringUTFChars(remoteFileName, NULL);
    const char *saveFile = env->GetStringUTFChars(toSaveFileName, NULL);

    bool result = client->DownloadRemoteFile(pid,remoteFile,saveFile,toPlaySize);
    int returnValue = 0;
    if(!result){
        returnValue = -2;
    }

    env->ReleaseStringUTFChars(peerId,pid);
    env->ReleaseStringUTFChars(remoteFileName,remoteFile);
    env->ReleaseStringUTFChars(toSaveFileName,saveFile);
    return returnValue;
}

jint naSearchLocalDevice(JNIEnv *env, jobject thiz){
    if (localClient == NULL) {
        return -1;
    }
    localClient->SearchLocalDevice();
    return 0;
}

jint naConnectLocalDevice(JNIEnv *env, jobject thiz, jstring peerAddr){
    if (localClient == NULL) {
        return -1;
    }
    int ret = 0;
    const char * pid = env->GetStringUTFChars(peerAddr, NULL);
    if(!localClient->OpenTunnel(pid)){
        ret = -2;
    }
    env->ReleaseStringUTFChars(peerAddr,pid);
    return ret;
}

jint naDisconnectLocalDevice(JNIEnv *env, jobject thiz, jstring peerAddr){
    if (localClient == NULL) {
        return -1;
    }
    int ret = 0;
    const char * pid = env->GetStringUTFChars(peerAddr, NULL);
    if(!localClient->CloseTunnel(pid)){
        ret = -2;
    }
    env->ReleaseStringUTFChars(peerAddr,pid);
    return ret;
}

jint naStartLocalVideo(JNIEnv *env, jobject thiz, jstring peerAddr){
    if (localClient == NULL) {
        return -1;
    }
    int ret = 0;
    const char * pid = env->GetStringUTFChars(peerAddr, NULL);
    if(!localClient->StartPeerMedia(pid)){
        ret = -2;
    }
    env->ReleaseStringUTFChars(peerAddr,pid);
    return ret;
}

jint naStopLocalVideo(JNIEnv *env, jobject thiz, jstring peerAddr){
    if (localClient == NULL) {
        return -1;
    }
    int ret = 0;
    const char * pid = env->GetStringUTFChars(peerAddr, NULL);
    if(!localClient->StopPeerMedia(pid)){
        ret = -2;
    }
    env->ReleaseStringUTFChars(peerAddr,pid);
    return ret;
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
        { "naMessageFromPeer", "(Ljava/lang/String;Ljava/lang/String;)I",
          (void*) naMessageFromPeer },
        { "naSendTalkData", "([BI)I", (void*) naSendTalkData },
        { "naStartPeerVideoCut", "(Ljava/lang/String;Ljava/lang/String;)I",
          (void*) naStartPeerVideoCut },
        { "naStopPeerVideoCut", "(Ljava/lang/String;)I", (void*) naStopPeerVideoCut },
        { "naDownloadRemoteFile", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)I",
          (void*) naDownloadRemoteFile },
        { "naSearchLocalDevice","()I",(void*) naSearchLocalDevice},
        { "naConnectLocalDevice", "(Ljava/lang/String;)I", (void*) naConnectLocalDevice },
        { "naDisconnectLocalDevice", "(Ljava/lang/String;)I", (void*) naDisconnectLocalDevice },
        { "naStartLocalVideo", "(Ljava/lang/String;)I", (void*) naStartLocalVideo },
        { "naStopLocalVideo", "(Ljava/lang/String;)I", (void*) naStopLocalVideo },

    };

    jclass cls = env->FindClass("com/video/play/TunnelCommunication");
    env->RegisterNatives(cls, nm, NELEM(nm));

    JniUtil::GetInstance()->g_vm_ = pVm;

    return JNI_VERSION_1_6;
}
