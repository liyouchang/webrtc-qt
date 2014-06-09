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

    void on_ptz_up_pressed();

    void on_ptz_up_released();

    void on_ptz_left_pressed();

    void on_ptz_left_released();

    void on_ptz_down_pressed();

    void on_ptz_down_released();

    void on_ptz_right_pressed();

    void on_ptz_right_released();


    void on_video2_clicked();

    void on_getwifi_clicked();

    void on_stop_cut_clicked();

    void on_searchDevice_clicked();

    void on_connectLocal_clicked();

    void on_disconnectLocal_clicked();

    void on_startLocalMedia_clicked();

    void on_stopLocalMedia_clicked();

private:
    void OnPtzMove(std::string side);
    void OnVideoClarity(int clarity);

    Ui::TunnelClientUI *ui;
    //PeerTerminal * terminal_;
//    talk_base::scoped_ptr<KeQtTunnelClient> tunnel_;
//    talk_base::scoped_ptr<PeerConnectionClientDealer> connection_;

   // QFile * recordFile;

    // PeerTerminalObserver interface
public:
};

#endif // TUNNELCLIENTUI_H
