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
    //

    return a.exec();
}

TunnelClientUI::TunnelClientUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TunnelClientUI)
{
    ui->setupUi(this);

   // "[{"uri":"stun:222.174.213.185:5389"},{"uri":"turn:222.174.213.185:5766"}]"
    QString configStr = "";
    Json::StyledWriter writer;

    Json::Value jmessage;
    Json::Value stunMsg;
    stunMsg["uri"] = "stun:192.168.40.179:5389";
    jmessage.append(stunMsg);
    std::string msg = writer.write(jmessage);

    this->ui->playPlugin->Initialize("tcp://192.168.40.179:5555",msg.c_str());

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

    this->ui->playPlugin->StartVideo(peer_id,1);
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



void TunnelClientUI::on_video2_clicked()
{
    QString peer_id = ui->edit_peer_id->text();

    this->ui->playPlugin->StartVideo(peer_id,2);

}

void TunnelClientUI::on_getwifi_clicked()
{
    Json::StyledWriter writer;
    Json::Value jmessage;

    jmessage["type"] = "tunnel";
    jmessage["command"] = "wifi_info";

    std::string msg = writer.write(jmessage);
    QString peerId = ui->edit_peer_id->text();

    this->ui->playPlugin->SendCommand(peerId,msg.c_str());
}

void TunnelClientUI::on_stop_cut_clicked()
{
    QString peerId = ui->edit_peer_id->text();
    this->ui->playPlugin->StopCut(peerId);

}

void TunnelClientUI::on_searchDevice_clicked()
{
    this->ui->playPlugin->SearchLocalDevice();
}

void TunnelClientUI::on_connectLocal_clicked()
{
    QString peerAddr = ui->editAddr->text();
    this->ui->playPlugin->OpenLocalDevice(peerAddr);
}

void TunnelClientUI::on_disconnectLocal_clicked()
{
    QString peerAddr = ui->editAddr->text();
    this->ui->playPlugin->CloseLocalDevice(peerAddr);

}

void TunnelClientUI::on_startLocalMedia_clicked()
{
    QString peerAddr = ui->editAddr->text();
    this->ui->playPlugin->StartLocalVideo(peerAddr);

}

void TunnelClientUI::on_stopLocalMedia_clicked()
{
    QString peerAddr = ui->editAddr->text();
    this->ui->playPlugin->StopLocalVideo(peerAddr);

}
