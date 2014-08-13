#include "PlayWidget.h"
#include <QtWidgets>
#include "AVService.h"
PlayWidget::PlayWidget(int index, QWidget *parent) :
    QWidget(parent),m_playIndex(index)
{
    selected = false;
    playSource = new AVService(m_playIndex+1);

}

PlayWidget::~PlayWidget()
{
    delete playSource;
}
void PlayWidget::setSelected(bool select)
{
    this->selected = select;
    //this->update();
}

void PlayWidget::PlayMediaData(QByteArray &data)
{
    if(!playSource->IsPlaying()){
        qDebug()<<"PlayWidget::PlayMediaData";
        HWND playWId = (HWND)this->winId();
        if(playSource->OpenStream(playWId)!=0)
            return;
        playSource->OpenSound();
    }
    playSource->InputStream(data.constData(),data.length());

}

void PlayWidget::PlayFile(QString file_name,int file_size)
{
    QByteArray data = file_name.toLocal8Bit();
    HWND playWId = (HWND)this->effectiveWinId();
    playSource->PlayFile(data.constData(),file_size,playWId);
}

void PlayWidget::SetPlayFileSize(int size)
{
    playSource->SetFileSize(size);
}

void PlayWidget::StopPlayFile()
{
    playSource->CloseFile();
    this->update();
}

bool PlayWidget::OpenSound()
{
    int ret = playSource->OpenSound();
    //qDebug()<<"PlayWidget::OpenSound---"<<ret;
    return true;
}

bool PlayWidget::CloseSound()
{
    int ret = playSource->CloseSound();
    //qDebug()<<"PlayWidget::CloseSound---"<<ret;
    return true;
}

bool PlayWidget::IsPlaying()
{
    return playSource->IsPlaying();
}

void PlayWidget::StopPlay()
{
    playSource->CloseStream();
    this->update();
}

bool PlayWidget::Capture(QString file_name)
{
    QByteArray data = file_name.toLocal8Bit();
    int ret = playSource->CapPic(data.constData());
    return true;
}

void PlayWidget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    if(this->selected){
        p.setPen(Qt::yellow);
    }else{
        p.setPen(Qt::black);
    }
    QRect r(event->rect());
    r.adjust(0, 0, -1, -1);
    p.drawRect(r);
    //p.drawText(10,10,QString::number(playIndex()));
    QWidget::paintEvent(event);
}

