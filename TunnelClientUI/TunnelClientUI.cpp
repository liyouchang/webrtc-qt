#include "TunnelClientUI.h"
#include "ui_TunnelClientUI.h"


#include "talk/base/logging.h"
#include "talk/base/json.h"

#include <QApplication>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    TunnelClientUI w;
    w.show();
    //talk_base::LogMessage::ConfigureLogging("tstamp thread info debug",NULL);

    return a.exec();
}

TunnelClientUI::TunnelClientUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TunnelClientUI)
{
    ui->setupUi(this);
    this->ui->playPlugin->Initialize("tcp://192.168.40.191:5555");

    //connection_ = new   PeerConnectionClientDealer();
}

TunnelClientUI::~TunnelClientUI()
{
    delete ui;

}


void TunnelClientUI::on_btn_init_clicked()
{
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

void TunnelClientUI::on_btn_save_video_clicked()
{
    QString peerId = ui->edit_peer_id->text();
    this->ui->playPlugin->StartCut(peerId);
}

void TunnelClientUI::on_ptz_up_pressed()
{
    OnPtzMove("move_up");
}

void TunnelClientUI::on_ptz_up_released()
{
    OnPtzMove("stop");
}

void TunnelClientUI::OnPtzMove(std::string side)
{
    Json::StyledWriter writer;
    Json::Value jmessage;

    jmessage["type"] = "tunnel";
    jmessage["command"] = "ptz";
    jmessage["control"] = side;
    jmessage["param"] = 10;

    std::string msg = writer.write(jmessage);
    QString peerId = ui->edit_peer_id->text();

    this->ui->playPlugin->SendCommand(peerId,msg.c_str());

}

void TunnelClientUI::OnVideoClarity(int clarity)
{
    Json::StyledWriter writer;
    Json::Value jmessage;

    jmessage["type"] = "tunnel";
    jmessage["command"] = "video_clarity";
    jmessage["value"] = clarity;

    std::string msg = writer.write(jmessage);
    QString peerId = ui->edit_peer_id->text();

    this->ui->playPlugin->SendCommand(peerId,msg.c_str());

}

void TunnelClientUI::on_ptz_left_pressed()
{
    OnPtzMove("move_left");
}

void TunnelClientUI::on_ptz_left_released()
{
    OnPtzMove("stop");
}

void TunnelClientUI::on_ptz_down_pressed()
{
    OnPtzMove("move_down");

}

void TunnelClientUI::on_ptz_down_released()
{
    OnPtzMove("stop");

}

void TunnelClientUI::on_ptz_right_pressed()
{
    OnPtzMove("move_right");

}

void TunnelClientUI::on_ptz_right_released()
{
    OnPtzMove("stop");

}

void TunnelClientUI::on_clarity_low_clicked()
{
OnVideoClarity(1);
}

void TunnelClientUI::on_clarity_normal_clicked()
{
    OnVideoClarity(2);

}

void TunnelClientUI::on_pushButton_high_clicked()
{
    OnVideoClarity(3);

}

void TunnelClientUI::on_get_clarity_clicked()
{
    OnVideoClarity(101);

}
