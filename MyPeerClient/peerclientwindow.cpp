#include "peerclientwindow.h"
#include "ui_peerclientwindow.h"

PeerClientWindow::PeerClientWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PeerClientWindow)
{
    ui->setupUi(this);
}

PeerClientWindow::~PeerClientWindow()
{
    delete ui;
}
