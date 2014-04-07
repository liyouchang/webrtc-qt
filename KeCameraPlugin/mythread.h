#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>

class MyThread : public QThread
{
    Q_OBJECT
public:
    explicit MyThread(QObject *parent = 0);

signals:

public slots:
    void TestSlot();
    // QThread interface
protected:
    void run();
};

#endif // MYTHREAD_H
