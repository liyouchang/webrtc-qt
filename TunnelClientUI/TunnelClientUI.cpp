#include "TunnelClientUI.h"
#include "ui_TunnelClientUI.h"

TunnelClientUI::TunnelClientUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TunnelClientUI)
{
    ui->setupUi(this);
    terminal_ = new PeerTerminal();
    //terminal_->RegisterObserver(this);
}

TunnelClientUI::~TunnelClientUI()
{
    delete ui;
}


void TunnelClientUI::on_btn_init_clicked()
{
    terminal_->Initialize("tcp://192.168.0.182:5555","");

}

void TunnelClientUI::on_btn_connect_clicked()
{
    std::string peer_id = ui->edit_peer_id->text().toStdString();
    terminal_->ConnectToPeer(peer_id);
}

void TunnelClientUI::on_btn_video_clicked()
{

}

void TunnelClientUI::OnTunnelReadData(const char *peer_id, char *data, int len)
{
    LOG(INFO) <<__FUNCTION__<<"-- Read from "<<peer_id  <<" ,read  data "  << data;
    //VideoWall * v = static_cast<VideoWall *>(user);
   // QByteArray data(data,len);
    //v->MediaDataRecv(0,dataType,data);
}
