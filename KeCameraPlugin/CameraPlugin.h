#ifndef CAMERAPLUGIN_H
#define CAMERAPLUGIN_H

#include<QtWidgets>
#include <qtbrowserplugin.h>

#include <VideoWall.h>

class CameraPlugin : public QWidget
{
    Q_OBJECT

    Q_CLASSINFO("ClassID", "{F4418F4B-4A6B-4A93-948F-332025F395E8}")
    Q_CLASSINFO("InterfaceID", "{DDC0C3D6-E09B-493D-8C6C-12A9916C60E1}")
    Q_CLASSINFO("EventsID", "{F4C17E60-A7C0-44FE-98B3-D7DEACE50252}")

    Q_CLASSINFO("MIME", "application/camera-plugin:kcp:Kaer camera plugin")
    Q_CLASSINFO("ToSuperClass", "CameraPlugin")
    Q_CLASSINFO("DefaultProperty", "text")
public:
    explicit CameraPlugin(QWidget *parent = 0);
    ~CameraPlugin();
signals:
    void testSignal();
public slots:
    void SetDivision(int num);
    QString PlayLocalFile();
protected:
    VideoWall * videoWall;

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *);
    void moveEvent(QMoveEvent *);
    void resizeEvent(QResizeEvent *);


    // QWidget interface
protected:
    void mousePressEvent(QMouseEvent *);

    // QWidget interface
protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);
};




#endif // CAMERAPLUGIN_H
