#ifndef KETUNNELCLIENT_H
#define KETUNNELCLIENT_H

#include "KeMsgProcess.h"

namespace kaerp2p{

class RecorderAvi;
class RecordSaverInterface;

enum RecordStatus{
    kRecordStartPlay = 0,//请求录像下载成功
    kRecordPlayError = 1,//
    kRequestFileError = 2,//请求录像文件错误
    kRecordPlayEnd = 3,//播放结束
    kRecordPlaying = 4,//正在播放,返回播放位置和播放速度
    kRequestMsgError = 5//返回错误的消息
};
enum RemotePlayType{
    kRemotePlayStart = 1,
    kRemotePlayControl = 2,
    kRemotePlayStop = 3
};

class KeTunnelClient:public KeMsgProcessContainer{
    friend class KeMessageProcessClient;
public:
    KeTunnelClient();
    /**
     * @brief SendCommand
     * @param peer_id
     * @param command --- the message send to another peer
     * @return ---0 : success, 101 : command format error,
     */
    bool SendCommand(const std::string &peer_id,const std::string & command);
    /**
     * @brief StartPeerMedia
     * @param peer_id
     * @param video---1:main stream media,2:sub stream media
     * @return
     */
    virtual bool StartPeerMedia(std::string  peer_id,int video);
    virtual bool StopPeerMedia(std::string peer_id);
    virtual bool StartPeerTalk(std::string  peer_id,bool withListen = false);
    virtual bool StopPeerTalk(std::string  peer_id,bool withListen = false);
    virtual bool StartPeerVideoCut(const std::string &peer_id,
                                   const std::string & filename);
    virtual bool StopPeerVideoCut(const std::string & peer_id);
    //send talk data to camera
    sigslot::signal2<const char *, int > SignalTalkData;
    virtual void SendTalkData(const char * data,int len);
    virtual bool PlayRemoteFile(std::string  peerId,std::string remoteFileName);
    virtual bool SetPlayFileStatus(std::string peerId, int type, int position, int speed);

    virtual void OnTunnelOpened(PeerTerminalInterface * t,
                                const std::string & peer_id);
    virtual void OnRouterMessage(const std::string &peer_id,
                                 talk_base::Buffer &msg);
    virtual void OnRecvAudioData(const std::string & peer_id,
                                 const char * data,int len);
    virtual void OnRecvVideoData(const std::string & peer_id,
                                 const char * data,int len);
protected:
//    virtual void OnRecordFileData(const std::string & peer_id,
//                                  const char * data,int len);
    virtual void OnRecordStatus(const std::string & peer_id,int status,
                                int position,int speed);
    virtual void OnMediaStatus(const std::string &peer_id,int video,int audio,int talk);
};


class KeMessageProcessClient: public KeMsgProcess
{
public:
    KeMessageProcessClient(std::string peer_id,KeTunnelClient * container);
    virtual ~KeMessageProcessClient();
    virtual void AskVideo(int video, int listen, int talk);
    bool ReqestPlayFile(const char *remoteFile);
    void OnTalkData(const char * data,int len);
    bool StartVideoCut(const std::string &filename);
    bool StopVideoCut();
    void SetPlayFileStatus(int type, int position , int speed);
    sigslot::signal3<const std::string &,const char *,int > SignalRecvVideoData;
    sigslot::signal3<const std::string &,const char *,int > SignalRecvAudioData;
//    sigslot::signal3<const std::string &,const char *,int > SignalRecvFileData;
//    sigslot::signal2<const std::string &,int,int> SignalRecordPlayStatus;
protected:
    virtual void OnMessageRespond(talk_base::Buffer & msgData);
    virtual void RecvVideoData(talk_base::Buffer & msgData);
    virtual void RecvAudioData(talk_base::Buffer & msgData);
    virtual void OnRecvRecordMsg(talk_base::Buffer & msgData);
    virtual void RecvAskMediaResp(talk_base::Buffer & msgData);
    virtual void RecvPlayFileResp(talk_base::Buffer & msgData);
    virtual void RecvMediaStatus(talk_base::Buffer &msgData);
private:
    //RecorderAvi *cutter_;
    RecordSaverInterface *recordSaver;
    std::string requestReocrdFileName;
    int talk_status;
};

}
#endif // KETUNNELCLIENT_H
