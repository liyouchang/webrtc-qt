#include "CameraPlugin.h"


#include "mythread.h"


CameraPlugin::CameraPlugin(QWidget *parent) :
    QWidget(parent)
{

    qDebug("CameraPlugin::CameraPlugin");
    videoWall = new VideoWall(this);

   QTimer::singleShot(0,this,SLOT(PlayLocalFile()));

}

CameraPlugin::~CameraPlugin()
{
    qDebug("CameraPlugin::~CameraPlugin");
}


void CameraPlugin::SetDivision(int num)
{
    this->videoWall->SetDivision((VideoWall::ScreenDivisionType)num);
}

QString CameraPlugin::PlayLocalFile()
{
    emit testSignal();
    //QMessageBox::aboutQt(this);
   // this->videoWall->PlayLocalFile();
    return "play success";
}

void CameraPlugin::paintEvent(QPaintEvent *)
{
    qDebug()<<"CameraPlugin::paintEvent";
    QRect r(rect());
    this->videoWall->setGeometry(r);
}

void CameraPlugin::moveEvent(QMoveEvent *e)
{

}

void CameraPlugin::resizeEvent(QResizeEvent *e)
{
    qDebug()<<"CameraPlugin::resizeEvent";
    QRect r(rect());
    this->videoWall->setGeometry(r);
}

void CameraPlugin::mousePressEvent(QMouseEvent *)
{
    qDebug()<<"CameraPlugin::mousePressEvent";
}

bool CameraPlugin::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
//    if(eventType == "windows_generic_MSG"){
//        MSG *msg = (MSG *)message;
//        if(msg->message == WM_LBUTTONDOWN){
//            QWidget *focusWidget = QWidget::find((WId)msg->hwnd);
//            QEvent override(QEvent::MouseButtonPress);
//            QApplication::sendEvent(focusWidget,&override);
//            //QRect r(rect());
//            //this->videoWall->setGeometry(r);
//            return false;
//        }

//    }
    return false;
}

QTNPFACTORY_BEGIN("Trivial Qt-based Plugin", "A Qt-based LiveConnected plug-in that does nothing")
    QTNPCLASS(CameraPlugin)
QTNPFACTORY_END()

#ifdef QAXSERVER
#include <ActiveQt/QAxFactory>

QAXFACTORY_BEGIN("{332B88AB-9EED-48BF-995C-F9DEC09DAC53}", "{FF2B2757-E552-4DA1-9E3C-93BC40EF7EC3}")
    QAXCLASS(CameraPlugin)
QAXFACTORY_END()
#endif


