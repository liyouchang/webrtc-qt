#include "mainwindow.h"
#include <math.h>

#include "talk/base/common.h"
#include "talk/base/logging.h"
#include "defaults.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui_(CONNECT_TO_SERVER)
{
    //ui->setupUi(this);
    //connect UI build
    connectUI_ = new QWidget(this);
    QLayout * connectLayout = new QHBoxLayout(connectUI_);
    connectUI_->setLayout(connectLayout);

    connectLayout->addWidget(new QLabel("Server",connectUI_));

    QLineEdit *edit_ip = new QLineEdit(connectUI_);
    edit_ip->setObjectName(QStringLiteral("edit_ip"));

    edit_ip->setText( "218.56.11.182");
    QSizePolicy spIP(QSizePolicy::Preferred, QSizePolicy::Fixed);
    spIP.setHorizontalStretch(3);
    edit_ip->setSizePolicy(spIP);
    connectLayout->addWidget(edit_ip);

    connectLayout->addWidget(new QLabel(":",connectUI_));

    QLineEdit *edit_port = new QLineEdit(connectUI_);
    edit_port->setObjectName(QStringLiteral("edit_port"));
    edit_port->setText( "8888");
    QSizePolicy spPort(QSizePolicy::Preferred, QSizePolicy::Fixed);
    spPort.setHorizontalStretch(1);
    edit_port->setSizePolicy(spPort);
    connectLayout->addWidget(edit_port);

    QPushButton *btn_Connect = new QPushButton(connectUI_);
    btn_Connect->setObjectName(QStringLiteral("btn_Connect"));
    btn_Connect->setText(QApplication::translate("MainWindow", "Connect", 0));
    QObject::connect(btn_Connect,&QPushButton::clicked,this,&MainWindow::OnConnectServer);
    connectLayout->addWidget(btn_Connect);

    //peer list ui
    peerListUI_ = new QTreeWidget(this);
    peerListUI_->header()->setVisible(false);
    QObject::connect(peerListUI_,&QTreeWidget::itemDoubleClicked,this,&MainWindow::OnItemDoubleClicked);
    //peerListUI_->hide();

    //stream ui
    streamUI_ = new StreamingUIWidget(this);
    streamUI_->setMinimumSize(640,480);
    //streamUI_->hide();

    QWidget *newCenter = new QWidget(this);
    QVBoxLayout * centerLayout = new QVBoxLayout(newCenter);
    centerLayout->addWidget(connectUI_);
    centerLayout->addWidget(peerListUI_);
    centerLayout->addWidget(streamUI_);

    this->setCentralWidget(newCenter);

    QObject::connect(this,&MainWindow::signalUIThreadCallback,
                     this,&MainWindow::OnUIThreadCallback,
                     Qt::QueuedConnection);



}

MainWindow::~MainWindow()
{
}

void MainWindow::OnConnectServer()
{
    if (!callback_)
        return;
    if (ui_ != CONNECT_TO_SERVER)
        return;

    QLineEdit *edit_ip = connectUI_->findChild<QLineEdit *>("edit_ip");
    QLineEdit *edit_port = connectUI_->findChild<QLineEdit *>("edit_port");

    std::string server(edit_ip->text().toStdString());
    int port = edit_port->text().toInt();
    callback_->StartLogin(server, port);

    connectUI_->setEnabled(false);
}


void MainWindow::OnItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (!callback_)
        return;
    if (ui_ != LIST_PEERS)
        return;
    QVariant itemData = item->data(column,Qt::UserRole);
    bool ok;
    int peer_id = itemData.toInt(&ok);
    if(ok){
        callback_->ConnectToPeer(peer_id);
    }
}

void MainWindow::OnUIThreadCallback(int msgID, void *data)
{
    callback_->UIThreadCallback(msgID,data);
}



void MainWindow::RegisterObserver(MainWndCallback *callback)
{
    this->callback_ = callback;
}

bool MainWindow::IsWindow()
{
    return this->isEnabled();
}

void MainWindow::ShowMessageBox(const char *caption, const char *text, bool is_error)
{
    if(is_error)
    {
        QMessageBox::critical(this,caption,text);
    }
    else
    {
        QMessageBox::information(this,caption,text);
    }
}

MainWindowInterface::UI MainWindow::current_ui()
{
    return ui_;
}

void MainWindow::SwitchToConnectUI()
{
    //peerListUI_->hide();
    // streamUI_->hide();
    ui_ = CONNECT_TO_SERVER;
    peerListUI_->clear();
    connectUI_->setEnabled(true);
    // connectUI_->show();
    // this->setCentralWidget(connectUI_);
}

void MainWindow::SwitchToPeerList(const Peers &peers)
{
    //connectUI_->hide();
    //streamUI_->hide();
    ui_ = LIST_PEERS;
    //peerListUI_->setVisible(true);
    //this->setCentralWidget(peerListUI_);

    peerListUI_->clear();
    QTreeWidgetItem * top0 = new QTreeWidgetItem(peerListUI_);
    top0->setText(0,"List of currently connected peers:");

    Peers::const_iterator i = peers.begin();
    for (; i != peers.end(); ++i){
        QTreeWidgetItem *child = new QTreeWidgetItem(top0);
        child->setText(0, i->second.c_str());
        child->setData(0,Qt::UserRole,QVariant(i->first));
    }
    top0->setExpanded(true);


}

void MainWindow::SwitchToStreamingUI()
{
    // peerListUI_->hide();
    //connectUI_->hide();
    ui_ = STREAMING;
    // streamUI_->show();
    //    this->setCentralWidget(streamUI_);
}

void MainWindow::StartLocalRenderer(webrtc::VideoTrackInterface *local_video)
{
    streamUI_->StartLocalRenderer(local_video);
}

void MainWindow::StopLocalRenderer()
{
    streamUI_->StopLocalRenderer();
}

void MainWindow::StartRemoteRenderer(webrtc::VideoTrackInterface *remote_video)
{
    streamUI_->StartRemoteRenderer(remote_video);
}

void MainWindow::StopRemoteRenderer()
{
    streamUI_->StopRemoteRenderer();
}

void MainWindow::QueueUIThreadCallback(int msg_id, void *data)
{
    //QTimer::singleShot(0,this,SLOT(OnUIThreadCallback(int,void*));
    emit signalUIThreadCallback(msg_id,data);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Escape:
        if (callback_) {
            if (ui_ == STREAMING) {
                callback_->DisconnectFromCurrentPeer();
            } else {
                callback_->DisconnectFromServer();
            }
        }
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::closeEvent(QCloseEvent *)
{
    if (callback_)
        callback_->Close();
}
