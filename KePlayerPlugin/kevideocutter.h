#ifndef KEVIDEOCUTTER_H
#define KEVIDEOCUTTER_H

#include <QObject>

class QFile;

class KeVideoCutter : public QObject
{
    Q_OBJECT
public:
    explicit KeVideoCutter(QObject *parent = 0);
    bool Init(QString fileName);
    bool IsOpen();
signals:

public slots:
    void OnVideoData(const char * data,int len);
    void OnAudioData(const char * data,int len);
private:
    QFile *saveFile;
};

#endif // KEVIDEOCUTTER_H
