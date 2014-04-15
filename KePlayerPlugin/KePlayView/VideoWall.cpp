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
        this->players[i] = new PlayWidget(i,this);
        this->players[i]->resize(0,0);
    }
    m_divType = ScreenDivision_None;
    m_selectedPlayer = 0;
    setSelectedPlayer(0);
    SetDivision(ScreenDivision_Six);

    setAcceptDrops(true);
}

VideoWall::~VideoWall()
{
    AVService::Cleanup();
}



void VideoWall::SetDivision(ScreenDivisionType divType)
{
    qDebug("Start SetDivision %d",divType);
    int splitNum = 0;
    if(this->m_divType == divType) return;


    for(int i=0;i<m_layoutList.size();i++){
        int index = m_layoutList.at(i);
        mainLayout->removeWidget(this->players[index]);
        this->players[index]->resize(0,0);
    }
    m_layoutList.clear();
    switch(divType){
    case ScreenDivision_One:
    case ScreenDivision_Four:
    case ScreenDivision_Six:
    {
        splitNum = divType;
        for(int i=0;i<splitNum;i++)
            m_layoutList<<i;
        break;
    }
    default:
        qWarning("VideoWall::SetDivision-----devision type error");
        return;
    }
    if(m_selectedPlayer >= splitNum){
        this->setSelectedPlayer(0);
    }
    m_divType = divType;
    this->BuildLayout();
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

void VideoWall::BuildLayout()
{
    int splitNum = m_layoutList.size();

    switch(m_divType){
    case ScreenDivision_One:
    {
        for(int i=0;i< splitNum;i++)
            mainLayout->addWidget(this->players[m_layoutList.at(i)]);
        break;
    }
    case ScreenDivision_Four:
    {
        for(int i=0;i<splitNum;i++){
            mainLayout->addWidget(players[m_layoutList.at(i)],i/2,i%2);
        }
        break;
    }
    case ScreenDivision_Six:
    {
        for(int i = 0; i< splitNum; i++){
            int index = m_layoutList.at(i);
            switch(i){
            case 0: mainLayout->addWidget(players[index],0,0,2,2);break;
            case 1: mainLayout->addWidget(players[index],0,2);break;
            case 2: mainLayout->addWidget(players[index],1,2);break;
            case 3: mainLayout->addWidget(players[index],2,0);break;
            case 4: mainLayout->addWidget(players[index],2,1);break;
            case 5: mainLayout->addWidget(players[index],2,2);break;
            }
        }
        break;
    }
    default:
        qWarning("VideoWall::BuildLayout-----devision type error");
        return;
    }
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
        //int ret = playSource->PlayFile(filename.toLatin1().constData(),0,this->players[0]->effectiveWinId());
        //qDebug()<<"play result:"<<ret;
        //   QMessageBox::information(this, "Document", "No document", QMessageBox::Ok | QMessageBox::Cancel);

    } else // 用户取消选择
    {

    }

}

void VideoWall::ExchangePlayWidget(int from, int to)
{
    qDebug()<<"VideoWall::ExchangePlayWidget-- from" << from<<" to "<<to;
    if(from == to){
        return;
    }
    int fromLayoutIndex = m_layoutList.indexOf(from);
    int toLayouIndex = m_layoutList.indexOf(to);

    m_layoutList[fromLayoutIndex] = to;
    m_layoutList[toLayouIndex] = from;

    BuildLayout();

}


void VideoWall::setSelectedPlayer(int newSelected)
{
    this->players[m_selectedPlayer]->setSelected(false);
    this->m_selectedPlayer = newSelected;
    this->players[m_selectedPlayer]->setSelected(true);
}

void VideoWall::deleteItem()
{
    qDebug("VideoWall::deleteItem %d",m_selectedPlayer);
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

    this->players[0]->PlayMediaData(data);


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
        //drag end
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
    //qDebug()<<"VideoWall::paintEvent "<<event->rect();
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
            performDrag(this->players[m_selectedPlayer]);
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
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
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

        int moveFrom;
//        QPoint offset;
        dataStream >> moveFrom;

        PlayWidget *child = static_cast<PlayWidget*>(this->childAt(event->pos()));
        if (!child){
            qDebug()<<"VideoWall::dropEvent--- no find PlayWidget";
            event->ignore();
            return;
        }

        int moveTo = child->playIndex();
        ExchangePlayWidget(moveFrom,moveTo);


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
