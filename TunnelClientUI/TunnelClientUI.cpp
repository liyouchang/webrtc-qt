#include "TunnelClientUI.h"
#include "ui_TunnelClientUI.h"



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
    terminal_ = new PeerTerminal();
    //terminal_->RegisterObserver(this);
}

TunnelClientUI::~TunnelClientUI()
{
    delete ui;

}


void TunnelClientUI::on_btn_init_clicked()
{
    //terminal_->Initialize("tcp://192.168.0.182:5555","");

    terminal_->Initialize("tcp://218.56.11.182:5555","");
}

void TunnelClientUI::on_btn_connect_clicked()
{
    std::string peer_id = ui->edit_peer_id->text().toStdString();
    terminal_->ConnectToPeer(peer_id);
    msg_processer_.reset( new KeMsgProcessClient());
    msg_processer_->SetTerminal(peer_id,terminal_);
    QObject::connect(msg_processer_.get(),&KeMsgProcessClient::SigRecvMediaData,
                     ui->videoWall,&VideoWall::OnRecvMediaData);
    QObject::connect(msg_processer_.get(),&KeMsgProcessClient::SigRecvMediaData,
                     this,&TunnelClientUI::OnRecvMediaData);

}

void TunnelClientUI::on_btn_video_clicked()
{
    msg_processer_->AskVideo();
}


void TunnelClientUI::on_btn_disconnect_clicked()
{
    terminal_->CloseTunnel();
}

void TunnelClientUI::OnRecvMediaData(int cameraID, int dataType, QByteArray data)
{
    static bool fileOpened = false;
    //qDebug()<<"id "<<cameraID<<" type:"<<dataType<<" datalen:"<<data.size();
    if(!fileOpened){
        this->recordFile = new QFile("SavedFileName.h264",this);
        if(!this->recordFile->open(QIODevice::WriteOnly)){
            qWarning("open save record file error");
            return;
        }
        fileOpened =true;
    }
    this->recordFile->write(data);
    this->recordFile->flush();

}
