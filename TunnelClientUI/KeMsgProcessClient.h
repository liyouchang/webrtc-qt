#ifndef KEMSGPROCESSCLIENT_H
#define KEMSGPROCESSCLIENT_H

#include <QObject>
#include "../zmqclient/KeMessage.h"
#include "../zmqclient/peerterminal.h"
class KeMsgProcessClient : public QObject,public sigslot::has_slots<>
{
    Q_OBJECT
public:
    explicit KeMsgProcessClient(QObject *parent = 0);
    void SetTerminal(std::string  peer_id,PeerTerminalInterface * t);
    int AskVideo();
    void OnTunnelMessage(const std::string & peer_id,talk_base::Buffer & msg);
signals:
    void SigRecvMediaData(int channelID,int mediaType,QByteArray data);

public slots:
    void ExtractMessage(QByteArray &allBytes);
    void OnMessageRespond(QByteArray & msgData);

protected:
    std::string peer_id_;
    PeerTerminalInterface * terminal_;
    QByteArray msgRecv;
    int bufPos; //witch
    char headBuf[11];
    int toRead;
    static const int msgMaxLen = 8192;

};

#endif // KEMSGPROCESSCLIENT_H
