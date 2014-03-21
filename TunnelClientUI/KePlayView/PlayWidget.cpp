#include "PlayWidget.h"

PlayWidget::PlayWidget(QMenu *contextMenu, QWidget *parent) :
    QWidget(parent)
{
    selected = false;
    myContextMenu = contextMenu;
}

void PlayWidget::setPlayIndex(int index)
{
    this->playIndex = index;
}

void PlayWidget::setSelected(bool select)
{
    this->selected = select;
    this->update();
}

void PlayWidget::paintEvent(QPaintEvent *)
{
    //qDebug("%d ,PlayWidget::paintEvent",playIndex);
    QPainter p(this);

    if(this->selected){
        p.setPen(Qt::yellow);

    }else{
        p.setPen(Qt::black);
    }
    QRect r(rect());
    r.adjust(0, 0, -1, -1);
    p.drawRect(r);

}

void PlayWidget::mouseReleaseEvent(QMouseEvent *)
{

}

void PlayWidget::mousePressEvent(QMouseEvent *)
{
    //qDebug("PlayWidget::mousePressEvent");
    emit selectPlayer(playIndex);
}

bool PlayWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    return false;
}

void PlayWidget::contextMenuEvent(QContextMenuEvent *event)
{
    //qDebug("PlayWidget::contextMenuEvent");

    emit selectPlayer(playIndex);
    if(myContextMenu)
        myContextMenu->exec(event->globalPos());
}
