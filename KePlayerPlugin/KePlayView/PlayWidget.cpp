#include "PlayWidget.h"
#include <QtWidgets>
#include "AVService.h"
PlayWidget::PlayWidget(int index, QWidget *parent) :
    QWidget(parent),m_playIndex(index)
{
    selected = false;
    m_playSource = new AVService(this,m_playIndex+1);

}
void PlayWidget::setSelected(bool select)
{
    this->selected = select;
    this->update();
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
    p.drawText(10,10,QString::number(playIndex()));
    qDebug()<<"PlayWidget::paintEvent "<<playIndex()<<" rect "<<r;
    QWidget::paintEvent(event);


}

//bool PlayWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
//{
//    return false;
//}

//void PlayWidget::contextMenuEvent(QContextMenuEvent *event)
//{
//    //qDebug("PlayWidget::contextMenuEvent");

//}

