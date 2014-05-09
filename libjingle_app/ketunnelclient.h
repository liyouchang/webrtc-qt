#ifndef KETUNNELCLIENT_H
#define KETUNNELCLIENT_H

#include "KeMsgProcess.h"


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
    int SendCommand(const std::string &peer_id,const std::string & command);

    /**
     * @brief StartPeerMedia
     * @param peer_id
     * @param video---0:stop media,1:main stream media,2:sub stream media
     * @return
     */
    virtual int StartPeerMedia( std::string  peer_id,int video);

    //send talk data to camera
    sigslot::signal2<const char *, int > SignalTalkData;
    virtual void SendTalkData(const char * data,int len);
    virtual bool DownloadRemoteFile(std::string  peer_id,
                                    std::string remote_file_name);
    virtual void OnTunnelOpened(PeerTerminalInterface * t,
                                const std::string & peer_id);
    virtual void OnRouterMessage(const std::string &peer_id,
                                 const std::string& msg);

protected:
    virtual void OnRecvAudioData(const std::string & peer_id,
                                 const char * data,int len);
    virtual void OnRecvVideoData(const std::string & peer_id,
                                 const char * data,int len);
    virtual void OnRecordFileData(const std::string & peer_id,
                                  const char * data,int len);
    virtual void OnRecordStatus(const std::string & peer_id,int status);
};


class KeMessageProcessClient: public KeMsgProcess
{
public:
    KeMessageProcessClient(std::string peer_id,KeTunnelClient * container);

    void AskVideo(int video, int listen, int talk);
    void ReqestPlayFile(const char * file_name);
    void OnTalkData(const char * data,int len);

    sigslot::signal3<const std::string &,const char *,int > SignalRecvVideoData;
    sigslot::signal3<const std::string &,const char *,int > SignalRecvAudioData;
    sigslot::signal3<const std::string &,const char *,int > SignalRecvFileData;
    sigslot::signal2<const std::string &,int > SignalRecordPlayStatus;

protected:
    virtual void OnMessageRespond(talk_base::Buffer & msgData);
    virtual void OnRecvRecordMsg(talk_base::Buffer & msgData);
    virtual void RecvMediaData(talk_base::Buffer & msgData);
};


#endif // KETUNNELCLIENT_H
