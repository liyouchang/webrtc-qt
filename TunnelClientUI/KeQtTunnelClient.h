#ifndef KEMSGPROCESSCLIENT_H
#define KEMSGPROCESSCLIENT_H

#include <QObject>
#include "../libjingle_app/KeMsgProcessContainer.h"
#include "libjingle_app/KeMsgProcessContainer.h"
class KeQtTunnelClient : public QObject,public KeTunnelClient
{
    Q_OBJECT
public:
    explicit KeQtTunnelClient(QObject *parent = 0);
signals:
    void SigRecvVideoData(QString peer_id,QByteArray data);
    void SigRecvAudioData(QString peer_id,QByteArray data);

protected:
    void OnRecvAudioData(const std::string & peer_id,const char * data,int len);
    void OnRecvVideoData(const std::string & peer_id,const char * data,int len);

};

#endif // KEMSGPROCESSCLIENT_H
