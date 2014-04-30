#ifndef TUNNELCLIENTUI_H
#define TUNNELCLIENTUI_H

#include <QMainWindow>
#include "../zmqclient/peerconnectionclientdealer.h"
#include <QFile>
#include "KeQtTunnelClient.h"
namespace Ui {
class TunnelClientUI;
}

class TunnelClientUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit TunnelClientUI(QWidget *parent = 0);
    ~TunnelClientUI();

private slots:

    void on_btn_init_clicked();

    void on_btn_connect_clicked();

    void on_btn_video_clicked();

    void on_btn_disconnect_clicked();
    //save to file
    void OnRecvMediaData(int cameraID,int dataType,QByteArray data);

    void on_btn_stop_video_clicked();

    void on_btn_save_video_clicked();

private:
    Ui::TunnelClientUI *ui;
    //PeerTerminal * terminal_;
//    talk_base::scoped_ptr<KeQtTunnelClient> tunnel_;
//    talk_base::scoped_ptr<PeerConnectionClientDealer> connection_;

   // QFile * recordFile;

    // PeerTerminalObserver interface
public:
};

#endif // TUNNELCLIENTUI_H
