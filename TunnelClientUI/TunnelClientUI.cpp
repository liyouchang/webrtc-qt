#include "TunnelClientUI.h"
#include "ui_TunnelClientUI.h"


#include "talk/base/logging.h"
#include <QApplication>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    TunnelClientUI w;
    w.show();
    talk_base::LogMessage::ConfigureLogging("tstamp thread info debug",NULL);

    return a.exec();
}

TunnelClientUI::TunnelClientUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TunnelClientUI)
{
    ui->setupUi(this);
    //connection_ = new   PeerConnectionClientDealer();
}

TunnelClientUI::~TunnelClientUI()
{
    delete ui;

}


void TunnelClientUI::on_btn_init_clicked()
{
    this->ui->playPlugin->Initialize("tcp://192.168.40.191:5555");
    //connection_->Connect("tcp://192.168.0.182:5555","");
//    connection_->Connect("tcp://192.168.40.191:5555","");
//    tunnel_->Initialize(connection_.get());
}

void TunnelClientUI::on_btn_connect_clicked()
{
    QString peer_id = ui->edit_peer_id->text();
    this->ui->playPlugin->OpenTunnel(peer_id);

//
//    tunnel_->ConnectToPeer(peer_id);
//    QObject::connect(tunnel_.get(),&KeQtTunnelClient::SigRecvVideoData,
//                     ui->videoWall,&VideoWall::OnRecvMediaData);
//    QObject::connect(tunnel_.get(),&KeQtTunnelClient::SigRecvAudioData,
//                     ui->videoWall,&VideoWall::OnRecvMediaData);


}

void TunnelClientUI::on_btn_video_clicked()
{
    QString peer_id = ui->edit_peer_id->text();

    this->ui->playPlugin->StartVideo(peer_id);
//    std::string peer_id = ui->edit_peer_id->text().toStdString();
//    tunnel_->AskPeerVideo(peer_id);
}


void TunnelClientUI::on_btn_disconnect_clicked()
{


    QString peer_id = ui->edit_peer_id->text();
    this->ui->playPlugin->CloseTunnel(peer_id);

//    tunnel_->CloseTunnel(peer_id);
}

void TunnelClientUI::OnRecvMediaData(int cameraID, int dataType, QByteArray data)
{
//    static bool fileOpened = false;
//    //qDebug()<<"id "<<cameraID<<" type:"<<dataType<<" datalen:"<<data.size();
//    if(!fileOpened){
//        this->recordFile = new QFile("SavedFileName.h264",this);
//        if(!this->recordFile->open(QIODevice::WriteOnly)){
//            qWarning("open save record file error");
//            return;
//        }
//        fileOpened =true;
//    }
//    this->recordFile->write(data);
//    this->recordFile->flush();

}

void TunnelClientUI::on_btn_stop_video_clicked()
{
    QString peer_id = ui->edit_peer_id->text();

    this->ui->playPlugin->StopVideo(peer_id);

}
