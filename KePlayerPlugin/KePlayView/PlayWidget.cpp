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
    this->update();
}

void PlayWidget::PlayMediaData(QByteArray &data)
{
    if(!playSource->IsPlaying()){
        HWND playWId = (HWND)this->winId();
        if(playSource->OpenStream(playWId)!=0)
            return;
    }
    playSource->InputStream(data.constData(),data.length());

}

void PlayWidget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
//    qDebug()<<"PlayWidget::paintEvent ---"<<this->winId();

    if(this->selected){
        p.setPen(Qt::yellow);

    }else{
        p.setPen(Qt::black);
    }
    QRect r(event->rect());
    r.adjust(0, 0, -1, -1);
    p.drawRect(r);
    p.drawText(10,10,QString::number(playIndex()));
    //qDebug()<<"PlayWidget::paintEvent "<<playIndex()<<" rect "<<r;
    QWidget::paintEvent(event);


}

//void PlayWidget::showEvent(QShowEvent *)
//{
////    qDebug()<<"PlayWidget::showEvent ---"<<this->effectiveWinId();

//}

//bool PlayWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
//{
//    return false;
//}

//void PlayWidget::contextMenuEvent(QContextMenuEvent *event)
//{
//    //qDebug("PlayWidget::contextMenuEvent");

//}

