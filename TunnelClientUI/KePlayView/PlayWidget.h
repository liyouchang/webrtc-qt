#ifndef PLAYWIDGET_H
#define PLAYWIDGET_H

#include<QtWidgets>
#include <QWidget>

class PlayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlayWidget(QMenu *contextMenu,QWidget *parent = 0);
    void setPlayIndex(int index);
    void setSelected(bool select);
signals:
    void selectPlayer(int n);
public slots:

private:
    int playIndex;
    bool selected;
    QMenu *myContextMenu;

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
    void contextMenuEvent(QContextMenuEvent *event);
};

#endif // PLAYWIDGET_H
