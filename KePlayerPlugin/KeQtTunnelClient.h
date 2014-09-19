#ifndef KEMSGPROCESSCLIENT_H
#define KEMSGPROCESSCLIENT_H

#include <QObject>
#include "libjingle_app/ketunnelclient.h"
#include "libjingle_app/kelocalclient.h"

class KeQtTunnelClient : public QObject,public kaerp2p::KeTunnelClient
{
    Q_OBJECT
public:
    explicit KeQtTunnelClient(QObject *parent = 0);
    virtual void OnTunnelOpened(kaerp2p::PeerTerminalInterface *t, const std::string &peer_id);
    virtual void OnTunnelClosed(kaerp2p::PeerTerminalInterface *t, const std::string &peer_id);
    virtual void OnRouterMessage(const std::string &peer_id, talk_base::Buffer &msg);
public slots:
    void OnTalkData(QByteArray data);
signals:
    void SigRecvVideoData(QString peer_id,QByteArray data);
    void SigRecvAudioData(QString peer_id,QByteArray data);
    void SigTunnelOpened(QString peer_id);
    void SigTunnelClosed(QString peer_id);
    //void SigRecordFileData(QString peer_id,QByteArray data);
    void SigRecordStatus(QString peer_id,int status,int position, int speed);
    void SigRecvPeerMsg(QString peer_id,QString msg);
    void SigMediaStatus(QString peer_id,int video,int audio,int talk);
protected:
    void OnRecvAudioData(const std::string & peer_id,const char * data,int len);
    void OnRecvVideoData(const std::string & peer_id,const char * data,int len);
//    void OnRecordFileData(const std::string &peer_id, const char *data, int len);
    void OnRecordStatus(const std::string &peer_id, int status,int position, int speed);
    void OnMediaStatus(const std::string &peer_id,int video,int audio,int talk);

};

class KeQtLocalClient : public QObject ,public kaerp2p::KeLocalClient
{
    Q_OBJECT
public:
    explicit KeQtLocalClient(QObject *parent = 0);
    virtual void OnTunnelOpened(kaerp2p::PeerTerminalInterface *t, const std::string &peer_id);
    virtual void OnTunnelClosed(kaerp2p::PeerTerminalInterface *t, const std::string &peer_id);

signals:
    void SigRecvVideoData(QString peer_id,QByteArray data);
    void SigRecvAudioData(QString peer_id,QByteArray data);
    void SigTunnelOpened(QString peer_id);
    void SigTunnelClosed(QString peer_id);
    void SigSearchedDeviceInfo(QString devInfo);
protected:
    void OnRecvAudioData(const std::string & peer_id,const char * data,int len);
    void OnRecvVideoData(const std::string & peer_id,const char * data,int len);
    virtual void OnSearchedDeviceInfo(const std::string &devInfo);


};


#endif // KEMSGPROCESSCLIENT_H
