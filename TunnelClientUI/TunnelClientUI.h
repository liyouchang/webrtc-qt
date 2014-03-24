#ifndef TUNNELCLIENTUI_H
#define TUNNELCLIENTUI_H

#include <QMainWindow>
#include "../zmqclient/peerterminal.h"
#include "KeMsgProcessClient.h"

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

private:
    Ui::TunnelClientUI *ui;
    PeerTerminal * terminal_;
    talk_base::scoped_ptr<KeMsgProcessClient> msg_processer_;
    // PeerTerminalObserver interface
public:
};

#endif // TUNNELCLIENTUI_H
