#ifndef TUNNELCLIENTUI_H
#define TUNNELCLIENTUI_H

#include <QMainWindow>
#include "../zmqclient/peerterminal.h"
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

private:
    Ui::TunnelClientUI *ui;
    PeerTerminal * terminal_;

    // PeerTerminalObserver interface
public:
    void OnTunnelReadData(const char *peer_id, char *data, int len);
};

#endif // TUNNELCLIENTUI_H
