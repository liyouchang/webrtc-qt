#ifndef PEERCLIENTWINDOW_H
#define PEERCLIENTWINDOW_H

#include <QMainWindow>

namespace Ui {
class PeerClientWindow;
}

class PeerClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PeerClientWindow(QWidget *parent = 0);
    ~PeerClientWindow();

private:
    Ui::PeerClientWindow *ui;
};

#endif // PEERCLIENTWINDOW_H
