#include "ke_recorder.h"
#include <QDebug>
#include "talk/base/thread.h"


const int kLeastPlaySize = 100*1024;

KeRecorder::KeRecorder(QObject *parent) :
    QObject(parent),file_(NULL),sig_played(false),pre_file_size_(kLeastPlaySize)
{
}


bool KeRecorder::OpenFile(QString path, QString peer_id)
{
    if(file_ != NULL){
        delete file_;
        file_ = NULL;
    }
    save_peer_id_ = peer_id;
    save_file_path_ = path;
    file_ = new QFile(path,this);
    file_->open(QIODevice::ReadWrite);
    return true;
}

bool KeRecorder::OpenFile(RecordFileInfo info,QString record_path)
{
    if(file_ != NULL){
        delete file_;
        file_ = NULL;
    }
    pre_file_size_ = info.size;
    save_peer_id_ = info.peer_id.c_str();
    QString save_file_path_ =record_path +
            QString("%1-%2.h264").arg(save_peer_id_).arg(info.file_date.c_str());
    file_ = new QFile(save_file_path_,this);
    file_->open(QIODevice::ReadWrite);
    return true;
}

int KeRecorder::GetFileSize()
{
    return file_->size();
}

void KeRecorder::SetPreFileSize(int pre_size){
    pre_file_size_ = pre_size;
}

void KeRecorder::OnRecvRecordData(QString peer_id, QByteArray data)
{
    //ASSERT(peer_id == save_peer_id_);
    int ret = file_->write(data);
    if(ret < data.size()){
        qWarning()<<"file write size error";
        return;
    }
    //下载了一部分后，播放文件
    if(!sig_played &&file_->size() > kLeastPlaySize ){
        SigAbleToPlay(save_file_path_,pre_file_size_);
        sig_played = true;
    }

}

void KeRecorder::OnRecordDownloadEnd(QString peer_id)
{
    if(peer_id ==save_peer_id_){
        qDebug()<<"KeRecorder::OnRecordDownloadEnd delete"<<peer_id;

        delete this;
    }
}
