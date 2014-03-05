#ifndef P2PTHREAD_H
#define P2PTHREAD_H

#include <QThread>

class P2PThread : public QThread
{
    Q_OBJECT
public:
    explicit P2PThread(QObject *parent = 0);

signals:

public slots:


    // QThread interface
protected:
    void run();
};

#endif // P2PTHREAD_H
