#include <jni.h>

#include <string>
#include <iostream>
#include "JniPeerConnection.h"
#include "KeJniTunnelClient.h"
#include "JniUtil.h"
#include<pthread.h>

#include "talk/base/logging.h"

#include "libjingle_app/p2pconductor.h"
#include "libjingle_app/peerterminal.h"

KeJniTunnelClient * client = NULL;
JniPeerConnection * jniConnection = NULL;
KeJniLocalClient * localClient = NULL;
kaerp2p::PeerTerminal * terminal = NULL;
/*
 * @param jstrIceServers : the servers used by ice protocal to accomplish NAT traversal,
*  this param is a json string contains a array of json object of server connection info
*  (eg. "[{"uri":"stun:222.174.213.185:5389"},{"uri":"turn:222.174.213.185:5766"}]").
*/
jint naInitialize(JNIEnv *env, jobject thiz, jstring jstrIceServers) {
    LOGI("1. naInitialize()");
    env->GetJavaVM(&JniUtil::GetInstance()->g_vm_);
    JniUtil::GetInstance()->g_obj_ = env->NewGlobalRef(thiz);

    jniConnection = new JniPeerConnection();
    terminal = new kaerp2p::PeerTerminal(jniConnection);
    client = new KeJniTunnelClient();
    client->Init(terminal);

    kaerp2p::LocalTerminal * lt = new kaerp2p::LocalTerminal();
    lt->Initialize();
    localClient = new KeJniLocalClient();
    localClient->Init(lt);

    //set ice servers
    const char * iceservers = env->GetStringUTFChars(jstrIceServers, NULL);
    kaerp2p::P2PConductor::AddIceServers(iceservers);
    env->ReleaseStringUTFChars(jstrIceServers,iceservers);

    return 0;
}
jint naChangeIceServers(JNIEnv *env, jobject thiz, jstring jstrIceServers)
{
    const char * iceservers = env->GetStringUTFChars(jstrIceServers, NULL);
    kaerp2p::P2PConductor::AddIceServers(iceservers);
    env->ReleaseStringUTFChars(jstrIceServers,iceservers);
    return 0;
}

jint naTerminate(JNIEnv *env, jobject thiz)
{
    if (client) {
        delete client;
        client = NULL;
    }

    if(terminal){
        delete terminal;
        terminal = NULL;
    }
    if (jniConnection) {
        delete jniConnection;
        jniConnection = NULL;
    }

    if(localClient) {
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
    if (jniConnection == NULL) {
        return -1;
    }
    const char * pid = env->GetStringUTFChars(peer_id, NULL);
    const char * pMsg = env->GetStringUTFChars(message,NULL);
    jniConnection->OnMessageFromPeer(pid,pMsg);
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

jint naStartMediaData(JNIEnv *env, jobject thiz, jstring peer_id,jint level) {
    LOGI("5. naAskMediaData()");
    if (client == NULL) {
        return -1;
    }
    const char * pid = env->GetStringUTFChars(peer_id, NULL);
    client->StartPeerMedia(pid,level);
    env->ReleaseStringUTFChars(peer_id,pid);
    return 0;
}

jint naStopMediaData(JNIEnv *env, jobject thiz, jstring peer_id) {
    if (client == NULL) {
        return -1;
    }
    const char * pid = env->GetStringUTFChars(peer_id, NULL);
    client->StopPeerMedia(pid);
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

jint naStopPeerVideoCut(JNIEnv *env, jobject thiz,jstring peer_id)
{
    if (client == NULL){
        return -1;
    }
    const char *pid = env->GetStringUTFChars(peer_id, NULL);

    client->StopPeerVideoCut(pid);

    env->ReleaseStringUTFChars(peer_id,pid);
    return 0;
}

jint naPlayRemoteFile(JNIEnv *env, jobject thiz,jstring peerId,jstring remoteFileName)
{
    if (client == NULL){
        return -1;
    }
    const char *pid = env->GetStringUTFChars(peerId, NULL);
    const char *remoteFile = env->GetStringUTFChars(remoteFileName, NULL);

    bool result = client->PlayRemoteFile(pid,remoteFile);
    int returnValue = 0;
    if(!result){
        returnValue = -2;
    }

    env->ReleaseStringUTFChars(peerId,pid);
    env->ReleaseStringUTFChars(remoteFileName,remoteFile);
    return returnValue;
}
jint naStopRemoteFile(JNIEnv *env, jobject thiz,jstring peerId)
{
    if (client == NULL){
        return -1;
    }
    const char *pid = env->GetStringUTFChars(peerId, NULL);
    int returnValue = 0;
    if(!client->SetPlayFileStatus(pid,3,-1,-1)){
        returnValue = -2;
    }

    env->ReleaseStringUTFChars(peerId,pid);
    return 0;

}

jint naSetPlayPosition(JNIEnv *env, jobject thiz,jstring peerId,jint position){
    if (client == NULL){
        return -1;
    }
    const char *pid = env->GetStringUTFChars(peerId, NULL);

    int returnValue = 0;
    if(!client->SetPlayFileStatus(pid,2,position,-1)){
        returnValue = -2;
    }

    env->ReleaseStringUTFChars(peerId,pid);
    return returnValue;
}
//0-puase 100-continue
jint naSetPlaySpeed(JNIEnv *env, jobject thiz,jstring peerId,jint speed){
    if (client == NULL){
        return -1;
    }
    const char *pid = env->GetStringUTFChars(peerId, NULL);

    int returnValue = 0;
    if(!client->SetPlayFileStatus(pid,2,-1,speed)){
        returnValue = -2;
    }
    env->ReleaseStringUTFChars(peerId,pid);
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

//check whether the tunnel of peerId is opened
jboolean naIsTunnelOpened(JNIEnv *env, jobject thiz, jstring peer_id) {
    if (client == NULL) {
        return false;
    }
    const char * pid = env->GetStringUTFChars(peer_id, NULL);
    bool ret = client->IsTunnelOpened(pid);
    env->ReleaseStringUTFChars(peer_id,pid);
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
    LOG_F(INFO)<<" loading labrary";
    JNINativeMethod nm[] = {
        { "naInitialize", "(Ljava/lang/String;)I", (void*) naInitialize },
        { "naChangeIceServers", "(Ljava/lang/String;)I", (void*) naChangeIceServers },
        { "naTerminate", "()I", (void*) naTerminate },

        { "naOpenTunnel", "(Ljava/lang/String;)I", (void*) naOpenTunnel },
        { "naCloseTunnel", "(Ljava/lang/String;)I", (void*) naCloseTunnel },
        { "naStartMediaData", "(Ljava/lang/String;I)I", (void*) naStartMediaData },
        { "naStopMediaData", "(Ljava/lang/String;)I", (void*) naStopMediaData },
        { "naMessageFromPeer", "(Ljava/lang/String;Ljava/lang/String;)I",
          (void*) naMessageFromPeer },

        { "naSendTalkData", "([BI)I", (void*) naSendTalkData },

        { "naStartPeerVideoCut", "(Ljava/lang/String;Ljava/lang/String;)I",
          (void*) naStartPeerVideoCut },
        { "naStopPeerVideoCut", "(Ljava/lang/String;)I", (void*) naStopPeerVideoCut },

        { "naPlayRemoteFile", "(Ljava/lang/String;Ljava/lang/String;)I",
          (void*) naPlayRemoteFile },
        { "naStopRemoteFile", "(Ljava/lang/String;)I", (void*) naStopRemoteFile },
        { "naSetPlayPosition", "(Ljava/lang/String;I)I", (void*) naSetPlayPosition },
        { "naSetPlaySpeed", "(Ljava/lang/String;I)I", (void*) naSetPlaySpeed },

        { "naSearchLocalDevice","()I",(void*) naSearchLocalDevice},
        { "naConnectLocalDevice", "(Ljava/lang/String;)I", (void*) naConnectLocalDevice },
        { "naDisconnectLocalDevice", "(Ljava/lang/String;)I", (void*) naDisconnectLocalDevice },
        { "naStartLocalVideo", "(Ljava/lang/String;)I", (void*) naStartLocalVideo },
        { "naStopLocalVideo", "(Ljava/lang/String;)I", (void*) naStopLocalVideo },
        { "naIsTunnelOpened", "(Ljava/lang/String;)Z", (void*) naIsTunnelOpened }

    };

    jclass cls = env->FindClass("com/video/play/TunnelCommunication");
    env->RegisterNatives(cls, nm, NELEM(nm));

    JniUtil::GetInstance()->g_vm_ = pVm;

    return JNI_VERSION_1_6;
}
