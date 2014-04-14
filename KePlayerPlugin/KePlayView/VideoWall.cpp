#include "VideoWall.h"


#include <QtWidgets>

#include "PlayWidget.h"
#include "AVService.h"


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

    mainLayout = new QGridLayout(this);
    mainLayout->setSpacing(1);
    mainLayout->setMargin(1);
    this->setLayout(mainLayout);
    for(int i=0;i<MAX_AVPLAYER;i++){
        this->players[i] = new PlayWidget(this);
        this->players[i]->setPlayIndex(i);
    }
    oldDivType = ScreenDivision_None;
    selectedPlayer = 0;
    setSelectedPlayer(0);
    SetDivision(ScreenDivision_Six);

    //playSource = new AVService(this);

    setAcceptDrops(true);
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
        this->players[i]->resize(0,0);
    }


    switch(divType){
    case ScreenDivision_One:
    {
        splitNum = 1;
        //this->players[0]->show();
        mainLayout->addWidget(this->players[0]);
    }
        break;
    case ScreenDivision_Four:
    {
        splitNum = 4;
        for(int i=0;i<splitNum;i++){
            //this->players[i]->show();
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
        //        for(int i=0;i<splitNum;i++){
        //            this->players[i]->show();
        //        }
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

void VideoWall::performDrag(PlayWidget *pw)
{
    if(pw == NULL){
        return;
    }
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << pw->playIndex();

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-playwidget", itemData);


    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    if (drag->exec(Qt::CopyAction | Qt::MoveAction) == Qt::MoveAction) {
        //selectionModel()->clearSelection();
    }

    //    if (drag->exec(Qt::MoveAction | Qt::CopyAction, Qt::CopyAction) == Qt::MoveAction){
    //        qDebug()<<"VideoWall::mousePressEvent-- is Qt::MoveAction";
    //        //child->close();
    //    }
    //    else{
    //        qDebug()<<"VideoWall::mousePressEvent-- else";

    //        //child->show();
    //    }
}

void VideoWall::paintEvent(QPaintEvent *event)
{  
    //    QPainter p(this);
    //    QRect r(rect());
    //    p.setPen(Qt::black);
    //    r.adjust(0, 0, -1, -1);
    //    p.drawRect(r);
    qDebug()<<"VideoWall::paintEvent "<<event->rect();
    QWidget::paintEvent(event);
}

void VideoWall::contextMenuEvent(QContextMenuEvent *event)
{
    qDebug()<<"VideoWall::contextMenuEvent";
    PlayWidget *child = static_cast<PlayWidget*>(this->childAt(event->pos()));
    if (!child){
        qWarning()<<"VideoWall::contextMenuEvent--- no find PlayWidget";
        return;
    }
    int playIndex = child->playIndex();
    //this->setSelectedPlayer(playIndex);

    itemMenu->exec(event->globalPos());

}

void VideoWall::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        int distance = (event->pos() - startPos).manhattanLength();
        if (distance >= QApplication::startDragDistance()) {
            performDrag(this->players[selectedPlayer]);
        }
    }
}

void VideoWall::mousePressEvent(QMouseEvent *event)
{
    qDebug()<<"VideoWall::mousePressEvent";
    if (event->button() == Qt::LeftButton) {
        startPos = event->pos();
    }
    PlayWidget *child = static_cast<PlayWidget*>(this->childAt(event->pos()));
    if (!child){
        qDebug()<<"VideoWall::mousePressEvent--- no find PlayWidget";
    }else{
        int playIndex = child->playIndex();
        this->setSelectedPlayer(playIndex);
    }
    QWidget::mousePressEvent(event);
    //QPoint hotSpot = event->pos() - child->pos();



}

void VideoWall::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug()<<"VideoWall::dragEnterEvent";
    //! [4] //! [5]
    if (event->mimeData()->hasFormat("application/x-playwidget")) {
        if (children().contains(event->source())) {
            qDebug()<<"VideoWall::dragEnterEvent-- children().contains(event->source";

            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            qDebug()<<"VideoWall::dragEnterEvent-- event->acceptProposedAction();";

            event->acceptProposedAction();
            //! [5] //! [6]
        }
        //! [6] //! [7]
    }  else {
        event->ignore();
    }

}

void VideoWall::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-playwidget")) {
        if (children().contains(event->source())) {
            qDebug()<<"VideoWall::dragMoveEvent-- children().contains(event->source";

            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    }  else {
        event->ignore();
    }
}

void VideoWall::dragLeaveEvent(QDragLeaveEvent *event)
{
    qDebug()<<"VideoWall::dragLeaveEvent";
    QWidget::dragLeaveEvent(event);
}

void VideoWall::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-playwidget")) {
        const QMimeData *mime = event->mimeData();


        QByteArray itemData = mime->data("application/x-playwidget");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);

        int moveFromIndex;
        QPoint offset;
        dataStream >> moveFromIndex >> offset;
        qDebug()<<"VideoWall::dropEvent----move  from "<<moveFromIndex;


        PlayWidget *child = static_cast<PlayWidget*>(this->childAt(event->pos()));
        if (!child){
            qDebug()<<"VideoWall::dropEvent--- no find PlayWidget";
            event->ignore();
            return;
        }
        qDebug()<<"VideoWall::dropEvent----move  to "<<child->playIndex();



        //        DragLabel *newLabel = new DragLabel(text, this);
        //        newLabel->move(event->pos() - offset);
        //        newLabel->show();
        //        newLabel->setAttribute(Qt::WA_DeleteOnClose);

        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
        //! [11] //! [12]
    } else {
        event->ignore();
    }
}

void VideoWall::keyPressEvent(QKeyEvent *e)
{

    if(e->key() == Qt::Key_Escape){
        showNormalScreenWall();
    }
}
