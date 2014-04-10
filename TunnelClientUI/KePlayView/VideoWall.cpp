#include "VideoWall.h"
#include "PlayWidget.h"

VideoWall::VideoWall(QWidget *parent) :
    QWidget(parent)
{

    AVService::Initialize();
    this->setBackgroundRole(QPalette::Dark);
    //this->setStyleSheet("background-color:rgb(255, 0, 0);");
    this->setAutoFillBackground(true);
    //QPalette palette;
    // palette.setColor(this->backgroundRole(), Qt::gray);
    //this->setPalette(palette);

    createActions();
    createMenus();

    mainLayout = new QGridLayout;
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    this->setLayout(mainLayout);
    for(int i=0;i<MAX_AVPLAYER;i++){
        this->players[i] = new PlayWidget(itemMenu,this);
        this->players[i]->setPlayIndex(i);
        QObject::connect(this->players[i],&PlayWidget::selectPlayer,this,&VideoWall::setSelectedPlayer);
    }
    oldDivType = ScreenDivision_None;
    selectedPlayer = 0;
    setSelectedPlayer(0);
    SetDivision(ScreenDivision_Six);

    playSource = new AVService(this);

    //with winID it will
   // playSource->SetPlayWnd((HWND)this->winId());
    qDebug()<<this->players[0]->effectiveWinId();
    //playSource->SetPlayWnd((HWND)this->players[0]->effectiveWinId());
}

VideoWall::~VideoWall()
{
    qDebug()<<this->players[0]->effectiveWinId();

    AVService::Cleanup();
}



void VideoWall::SetDivision(ScreenDivisionType divType)
{
    qDebug("Start SetDivision %d",divType);


    static int splitNum = 0;
    if(oldDivType == divType) return;

    for(int i=0;i<MAX_AVPLAYER;i++){
        mainLayout->removeWidget(this->players[i]);
        this->players[i]->hide();
    }

    switch(divType){
    case ScreenDivision_One:
    {
        splitNum = 1;
        this->players[0]->show();
        mainLayout->addWidget(this->players[0]);
    }
        break;
    case ScreenDivision_Four:
    {
        splitNum = 4;
        for(int i=0;i<splitNum;i++){
            this->players[i]->show();
            mainLayout->addWidget(players[i],i/2,i%2);
        }
    }
        break;
    case ScreenDivision_Six:
    {
        splitNum = 6;
        mainLayout->addWidget(players[0],0,0,2,2);
        mainLayout->addWidget(players[1],0,2);
        mainLayout->addWidget(players[2],1,2);
        mainLayout->addWidget(players[3],2,0);
        mainLayout->addWidget(players[4],2,1);
        mainLayout->addWidget(players[5],2,2);
        for(int i=0;i<splitNum;i++){
            this->players[i]->show();
        }
    }
        break;

    default:
        return;
    }
    if(selectedPlayer >= splitNum){
        this->setSelectedPlayer(0);
    }
    oldDivType = divType;

}

void VideoWall::SetDivision(int num)
{
    static const int AvailableScreenDivisionType[] =
    {ScreenDivision_One,ScreenDivision_Four,ScreenDivision_Six,
     ScreenDivision_Eight,ScreenDivision_Nine,ScreenDivision_Ten1,
     ScreenDivision_Ten2,ScreenDivision_Thirteen1,ScreenDivision_Thirteen2,
     ScreenDivision_Fourteen,ScreenDivision_Sixteen,ScreenDivision_Seventeen,
     ScreenDivision_Nineteen,ScreenDivision_Twentytwo,ScreenDivision_Twentyfive,
     ScreenDivision_Thirtytwo,ScreenDivision_Sixtyfour};

    int arrayLen = sizeof(AvailableScreenDivisionType)/sizeof(AvailableScreenDivisionType[0]);
    int find = 0;
    for(;find<arrayLen;++find){
        if(num == AvailableScreenDivisionType[find]) break;
    }
    if(find == arrayLen) return;
    SetDivision(static_cast<ScreenDivisionType>(num));
}

void VideoWall::PlayLocalFile()
{
    QString filename = QFileDialog::getOpenFileName(
                this,
                "Open Video File",
                QDir::currentPath(),
                "Video files (*.h264 *.264);;All files(*.*)");
    if (!filename.isNull()) { //用户选择了文件
        qDebug()<<QDir::currentPath();
        int ret = playSource->PlayFile(filename.toLatin1().constData(),0,this->players[0]->effectiveWinId());
         qDebug()<<"play result:"<<ret;
        //   QMessageBox::information(this, "Document", "No document", QMessageBox::Ok | QMessageBox::Cancel);

    } else // 用户取消选择
    {

    }

}


void VideoWall::setSelectedPlayer(int newSelected)
{
    this->players[selectedPlayer]->setSelected(false);
    this->selectedPlayer = newSelected;
    this->players[selectedPlayer]->setSelected(true);
}

void VideoWall::deleteItem()
{
    qDebug("VideoWall::deleteItem %d",selectedPlayer);
}

void VideoWall::showfullScreenWall()
{
    Qt::WindowFlags oldFlags = this->windowFlags();
    this->setWindowFlags(oldFlags | Qt::Dialog);
    this->showFullScreen();
}

void VideoWall::showNormalScreenWall()
{
    Qt::WindowFlags oldFlags = this->windowFlags();
    this->setWindowFlags(oldFlags & ~Qt::Dialog);
    this->showNormal();
}

void VideoWall::OnRecvMediaData(QString peer_id, QByteArray data)
{
    if(!playSource->IsPlaying()){
        WId playWId = this->players[0]->winId();
        if(playSource->OpenStream(playWId)!=0)
            return;
    }
    playSource->InputStream(data.constData(),data.length());

}

void VideoWall::createActions()
{
    deleteAction = new QAction(tr("&Delete"), this);
    deleteAction->setShortcut(tr("Delete"));
    deleteAction->setStatusTip(tr("Delete item from diagram"));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteItem()));

}

void VideoWall::createMenus()
{
    itemMenu = new QMenu(this);
    itemMenu->addAction(deleteAction);
    itemMenu->addSeparator();
}

void VideoWall::paintEvent(QPaintEvent *)
{  
//    qDebug()<<"VideoWall::paintEvent---player0 > "<<this->players[0]->effectiveWinId();
//    qDebug()<<"VideoWall::paintEvent---player1 > "<<this->players[1]->winId();
//    qDebug()<<"VideoWall::paintEvent---player2 > "<<this->players[2]->winId();

//    QPainter p(this);
//    QRect r(rect());
//    p.setPen(Qt::black);
//    r.adjust(0, 0, -1, -1);
//    p.drawRect(r);
}

void VideoWall::keyPressEvent(QKeyEvent *e)
{

    if(e->key() == Qt::Key_Escape){
        showNormalScreenWall();
    }
}
