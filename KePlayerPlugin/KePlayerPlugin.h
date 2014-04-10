#ifndef KEPLAYERPLUGIN_H
#define KEPLAYERPLUGIN_H

#include <QWidget>
#include <QtWidgets>
#include "VideoWall.h"
#include "zmqclient/peerconnectionclientdealer.h"
#include "TunnelClientUI/KeQtTunnelClient.h"

class KePlayerPlugin : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("ClassID", "{806873E8-893C-402D-8129-34951563272A}");
    Q_CLASSINFO("InterfaceID", "{63314D1F-5592-4166-87F0-655F2C8DBFF8}");
    Q_CLASSINFO("EventsID", "{45133EE9-DFB6-49FE-9D91-434ECCF96A49}");


    Q_CLASSINFO("MIME", "application/keplayer-plugin:kpp:Kaer player plugin")
    Q_CLASSINFO("ToSuperClass", "KePlayerPlugin")

public:
    KePlayerPlugin(QWidget *parent = 0);
    ~KePlayerPlugin();
private:
    VideoWall * videoWall;
    talk_base::scoped_ptr<KeQtTunnelClient> tunnel_;
    talk_base::scoped_ptr<PeerConnectionClientDealer> connection_;

public slots:

    void about();
    void SetDivision(int num);
    QString PlayLocalFile();
    int Initialize(QString routerUrl);
    int Connect(QString peer_id);
//    int DisConnect(QString peer_id);
    int StartVideo(QString peer_id);
//    int StopVideo(QString peer_id);
signals:
    void TunnelOpened();

};

#endif // KEPLAYERPLUGIN_H
