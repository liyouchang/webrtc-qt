#ifndef KEMSGPROCESSCLIENT_H
#define KEMSGPROCESSCLIENT_H

#include <QObject>
#include "libjingle_app/KeMsgProcessContainer.h"
class KeQtTunnelClient : public QObject,public KeTunnelClient
{
    Q_OBJECT
public:
    explicit KeQtTunnelClient(QObject *parent = 0);
signals:
    void SigRecvVideoData(QString peer_id,QByteArray data);
    void SigRecvAudioData(QString peer_id,QByteArray data);
    void SigTunnelOpened(QString peer_id);
    void SigTunnelClosed(QString peer_id);
    void SigTunnelRecordStatus(QString peer_id,int );
protected:
    void OnRecvAudioData(const std::string & peer_id,const char * data,int len);
    void OnRecvVideoData(const std::string & peer_id,const char * data,int len);
public:
    void OnTunnelOpened(PeerTerminalInterface *t, const std::string &peer_id);
    void OnTunnelClosed(PeerTerminalInterface *t, const std::string &peer_id);
};

#endif // KEMSGPROCESSCLIENT_H
