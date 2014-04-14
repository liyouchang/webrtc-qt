#ifndef PLAYWIDGET_H
#define PLAYWIDGET_H

#include <QWidget>

class AVService;

QT_BEGIN_NAMESPACE
class QMouseEvent;
class QMenu;

QT_END_NAMESPACE

class PlayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlayWidget(int index,QWidget *parent = 0);
    void setSelected(bool select);
    int playIndex() {return m_playIndex;}
    AVService * m_playSource;
signals:
    //void selectPlayer(int n);
public slots:

private:
    int m_playIndex;
    bool selected;
    // QWidget interface
protected:
    void paintEvent(QPaintEvent *);
//    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
   // void contextMenuEvent(QContextMenuEvent *event);
};

#endif // PLAYWIDGET_H
