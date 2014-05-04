#ifndef KEMSGPROCESSCONTAINER_H
#define KEMSGPROCESSCONTAINER_H

#include "PeerTerminalInterface.h"
#include "PeerConnectionClinetInterface.h"
#include "talk/base/messagehandler.h"
#include <vector>

class KeMsgProcess;
class KeMessageProcessCamera;
class KeMessageProcessClient;

class KeMsgProcessContainer: public sigslot::has_slots<>
{
    friend class KeMsgProcess;
public:
    KeMsgProcessContainer();
    virtual ~KeMsgProcessContainer();

    virtual bool Init(PeerTerminalInterface *t);
    virtual bool Init(kaerp2p::PeerConnectionClientInterface * client);
    virtual int OpenTunnel(const std::string &peer_id);
    virtual int CloseTunnel(const std::string &peer_id);
    virtual bool IsTunnelOpened(const std::string &peer_id);
    virtual void OnTunnelOpened(PeerTerminalInterface * t,
                                const std::string & peer_id);
    virtual void OnTunnelClosed(PeerTerminalInterface * t,
                                const std::string & peer_id);
    virtual void OnTunnelMessage(const std::string &peer_id,
                                 talk_base::Buffer &msg);
    virtual void OnRouterMessage(const std::string &peer_id,
                                 const std::string& msg);

protected:
    virtual KeMsgProcess * GetProcess(const std::string & peer_id);
    virtual void AddMsgProcess(KeMsgProcess * process);
    virtual void OnProcessNeedSend(const std::string & peer_id,
                                   const char * data,int len);
    virtual void OnHeartStop(const std::string & peer_id);

    std::vector<KeMsgProcess *> processes_;
    PeerTerminalInterface * terminal_;
    bool has_terminal;
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
    int SendCommand(const std::string &peer_id,const std::string & command);
    /**
     * @brief StartPeerMedia
     * @param peer_id
     * @param video---0:stop media,1:main stream media,2:sub stream media
     * @return
     */
    virtual int StartPeerMedia( std::string  peer_id,int video);

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

class KeTunnelCamera:public KeMsgProcessContainer{
    friend class KeMessageProcessCamera;
public:

    virtual void OnTunnelOpened(PeerTerminalInterface * t,
                                const std::string & peer_id);
    virtual void OnRouterMessage(const std::string &peer_id,
                                 const std::string &msg);
    virtual void SetVideoClarity(int);
    virtual int GetVideoClarity();
    virtual void SetPtz(std::string  ptz_key,int param);

protected:
    sigslot::signal2<const char *, int > SignalVideoData1;
    sigslot::signal2<const char *, int > SignalVideoData2;

    sigslot::signal2<const char *, int > SignalAudioData;
    virtual void OnRecvVideoClarity(std::string peer_id,int clarity);
    virtual void OnRecvRecordQuery(std::string peer_id, std::string condition);
    virtual void OnRecvGetWifiInfo(std::string peer_id);
    virtual void SetWifiInfo(std::string peer_id,std::string param);
    virtual void OnToPlayFile(const std::string &peer_id,
                              const std::string &filename);

};

#endif // KEMSGPROCESSCONTAINER_H
