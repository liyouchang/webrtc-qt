#ifndef KEMSGPROCESSCLIENT_H
#define KEMSGPROCESSCLIENT_H

#include <QObject>
#include "libjingle_app/KeMsgProcessContainer.h"

class KeQtTunnelClient : public QObject,public KeTunnelClient
{
    Q_OBJECT
public:
    explicit KeQtTunnelClient(QObject *parent = 0);
    virtual void OnTunnelOpened(PeerTerminalInterface *t, const std::string &peer_id);
    virtual void OnTunnelClosed(PeerTerminalInterface *t, const std::string &peer_id);
    virtual void OnRouterMessage(const std::string &peer_id, const std::string &msg);
public slots:
    bool StartVideoCut(QString peerId, QString filename);
    bool StopVideoCut(QString peerId);

signals:
    void SigRecvVideoData(QString peer_id,QByteArray data);
    void SigRecvAudioData(QString peer_id,QByteArray data);
    void SigTunnelOpened(QString peer_id);
    void SigTunnelClosed(QString peer_id);
    void SigRecordFileData(QString peer_id,QByteArray data);
    void SigRecordStatus(QString peer_id,int status);
    void SigRecvPeerMsg(QString peer_id,QString msg);

protected:
    void OnRecvAudioData(const std::string & peer_id,const char * data,int len);
    void OnRecvVideoData(const std::string & peer_id,const char * data,int len);
    void OnRecordFileData(const std::string &peer_id, const char *data, int len);
    void OnRecordStatus(const std::string &peer_id, int status);
};

#endif // KEMSGPROCESSCLIENT_H
