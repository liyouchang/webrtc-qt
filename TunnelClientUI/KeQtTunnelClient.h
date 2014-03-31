#ifndef KEMSGPROCESSCLIENT_H
#define KEMSGPROCESSCLIENT_H

#include <QObject>
#include "../libjingle_app/KeMsgProcessContainer.h"
class KeQtTunnelClient : public QObject,public KeMsgProcessContainer
{
    Q_OBJECT
public:
    explicit KeQtTunnelClient(QObject *parent = 0);
    int AskPeerVideo(std::string peer_id);
signals:
    void SigRecvVideoData(QString peer_id,QByteArray data);
    void SigRecvAudioData(QString peer_id,QByteArray data);

protected:
    void OnRecvAudioData(const std::string & peer_id,const char * data,int len);
    void OnRecvVideoData(const std::string & peer_id,const char * data,int len);

    // KeMsgProcessContainer interface
public:
    void OnTunnelOpened(PeerTerminalInterface *t, const std::string &peer_id);
};

#endif // KEMSGPROCESSCLIENT_H
