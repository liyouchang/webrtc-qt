#include "mythread.h"
#include<QTimer>
#include<QMessageBox>

MyThread::MyThread(QObject *parent) :
    QThread(parent)
{

}

void MyThread::TestSlot()
{
}

void MyThread::run()
{
    QTimer::singleShot(0,this,SLOT(TestSlot()));
    exec();
}
