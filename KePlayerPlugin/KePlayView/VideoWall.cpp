#include "VideoWall.h"


#include <QtWidgets>

//#include "PlayWidget.h"
#include "AVService.h"

const QString kLocalIndexPrefix = "local_file_";

VideoWall::VideoWall(QWidget *parent) :
    QWidget(parent),layout_changed_(false)
{
    AVService::Initialize();
    playSource = new AVService(1);

    this->setBackgroundRole(QPalette::Dark);
    //this->setStyleSheet("background-color:rgb(255, 0, 0);");
    this->setAutoFillBackground(true);
    //QPalette palette;
    // palette.setColor(this->backgroundRole(), Qt::gray);
    //this->setPalette(palette);
    qDebug()<<this->contentsMargins();
//    createActions();
//    createMenus();

//    mainLayout = new QGridLayout(this);
//    mainLayout->setSpacing(1);
//    mainLayout->setMargin(1);
//    this->setLayout(mainLayout);
//    for(int i=0;i<MAX_AVPLAYER;i++){
//        this->players[i] = new PlayWidget(i,this);
//        //hideLayout->addWidget( this->players[i]);
//        this->players[i]->hide();
//    }
//    m_divType = ScreenDivision_None;
    m_selectedPlayer = 0;
//    setSelectedPlayer(0);
//    SetDivision(ScreenDivision_One);


    talkTimer = new QTimer(this);
    QObject::connect(talkTimer, SIGNAL(timeout()), this, SLOT(OnGetTalkData()));


    // setAcceptDrops(true);
}

VideoWall::~VideoWall()
{
//    for(int i=0;i<MAX_AVPLAYER;i++){
//        delete this->players[i];
//    }
    delete playSource;
    AVService::Cleanup();
}



//void VideoWall::SetDivision(ScreenDivisionType divType)
//{
//    qDebug("Start SetDivision %d",divType);
//    int splitNum = 0;
//    if(this->m_divType == divType) return;
//    for(int i=0;i<m_layoutList.size();i++){
//        int index = m_layoutList.at(i);
//        mainLayout->removeWidget(this->players[index]);
//    }
//    m_layoutList.clear();
//    switch(divType){
//    case ScreenDivision_One:
//    case ScreenDivision_Four:
//    case ScreenDivision_Six:
//    {
//        splitNum = divType;
//        for(int i=0;i<splitNum;i++)
//            m_layoutList<<i;
//        break;
//    }
//    default:
//        qWarning("VideoWall::SetDivision-----devision type error");
//        return;
//    }
//    if(m_selectedPlayer >= splitNum){
//        this->setSelectedPlayer(0);
//    }
//    for(int i=0;i<splitNum;i++){
//        if(!this->players[i]->isVisible()){
//            this->players[i]->setVisible(true);
//        }
//    }
//    m_divType = divType;
//    this->BuildLayout();
//    for(int i=splitNum;i<MAX_AVPLAYER;i++){
//        emit SigNeedStopPeerPlay(GetPeerPlay(i));
//        this->players[i]->setVisible(false);
//        //this->players[i]->resize(0,0);
//    }
//    layout_changed_ = true;
//    this->update();
//}

//void VideoWall::SetDivision(int num)
//{
//    static const int AvailableScreenDivisionType[] =
//    {ScreenDivision_One,ScreenDivision_Four,ScreenDivision_Six,
//     ScreenDivision_Eight,ScreenDivision_Nine,ScreenDivision_Ten1,
//     ScreenDivision_Ten2,ScreenDivision_Thirteen1,ScreenDivision_Thirteen2,
//     ScreenDivision_Fourteen,ScreenDivision_Sixteen,ScreenDivision_Seventeen,
//     ScreenDivision_Nineteen,ScreenDivision_Twentytwo,ScreenDivision_Twentyfive,
//     ScreenDivision_Thirtytwo,ScreenDivision_Sixtyfour};

//    int arrayLen = sizeof(AvailableScreenDivisionType)/sizeof(AvailableScreenDivisionType[0]);
//    int find = 0;
//    for(;find<arrayLen;++find){
//        if(num == AvailableScreenDivisionType[find]) break;
//    }
//    if(find == arrayLen) return;
//    SetDivision(static_cast<ScreenDivisionType>(num));
//}

//void VideoWall::BuildLayout()
//{
//    int splitNum = m_layoutList.size();

//    switch(m_divType){
//    case ScreenDivision_One:
//    {
//        for(int i=0;i< splitNum;i++)
//            mainLayout->addWidget(this->players[m_layoutList.at(i)]);
//        break;
//    }
//    case ScreenDivision_Four:
//    {
//        for(int i=0;i<splitNum;i++){
//            mainLayout->addWidget(players[m_layoutList.at(i)],i/2,i%2);
//        }
//        break;
//    }
//    case ScreenDivision_Six:
//    {
//        for(int i = 0; i< splitNum; i++){
//            int index = m_layoutList.at(i);
//            switch(i){
//            case 0: mainLayout->addWidget(players[index],0,0,2,2);break;
//            case 1: mainLayout->addWidget(players[index],0,2);break;
//            case 2: mainLayout->addWidget(players[index],1,2);break;
//            case 3: mainLayout->addWidget(players[index],2,0);break;
//            case 4: mainLayout->addWidget(players[index],2,1);break;
//            case 5: mainLayout->addWidget(players[index],2,2);break;
//            }
//        }
//        break;
//    }
//    default:
//        qWarning("VideoWall::BuildLayout-----devision type error");
//        return;
//    }
//}

void VideoWall::PlayLocalFile(QString peer_id,QString file_name,int file_size)
{
    QString local_key = kLocalIndexPrefix + peer_id;
    int play_index = this->SetPeerPlay(local_key);
    //this->players[play_index]->PlayFile(file_name,file_size);
    QByteArray data = file_name.toLocal8Bit();
    HWND playWId = (HWND)this->effectiveWinId();
    playSource->PlayFile(data.constData(),file_size,playWId);
}



//void VideoWall::ExchangePlayWidget(int from, int to)
//{
//    qDebug()<<"VideoWall::ExchangePlayWidget-- from" << from<<" to "<<to;
//    if(from == to){
//        return;
//    }
//    int fromLayoutIndex = m_layoutList.indexOf(from);
//    int toLayouIndex = m_layoutList.indexOf(to);

//    m_layoutList[fromLayoutIndex] = to;
//    m_layoutList[toLayouIndex] = from;

//    BuildLayout();

//}

int VideoWall::SetPeerPlay(QString peer_id)
{
    int play_index = peer_play_map_.value(peer_id,-1);
    if(play_index == -1){
        QString played = GetPeerPlay(-1);
        if(!played.isEmpty()){
            emit SigNeedStopPeerPlay(played);
        }
        play_index = m_selectedPlayer;
        peer_play_map_.insert(peer_id,m_selectedPlayer);
        return play_index;//free to play
    }
    else {
        //this->setSelectedPlayer(play_index);
        return play_index;//has a player to play
    }

}

QString VideoWall::GetPeerPlay(int player_num)
{
    int play_index = m_selectedPlayer;
    if(player_num != -1){
        play_index = player_num;
    }
    return peer_play_map_.key(play_index);
}

void VideoWall::StopPeerPlay(QString peer_id)
{
//    qDebug()<<"VideoWall::StopPeerPlay "<<peer_id;
    int play_index = peer_play_map_.value(peer_id,-1);
    if(play_index != -1){
//        players[play_index]->StopPlay();
        playSource->CloseStream();
        this->update();
        peer_play_map_.insert(peer_id,-1);
    }

}

bool VideoWall::SetLocalPlayFileSize(QString peer_id,int size)
{
    QString local_key = kLocalIndexPrefix + peer_id;

    int play_index = peer_play_map_.value(local_key,-1);

    if(play_index != -1){
        //players[play_index]->SetPlayFileSize(size);
        playSource->SetFileSize(size);
        return true;
    }
    return false;
}

void VideoWall::StopFilePlay(QString peer_id)
{
    QString local_key = kLocalIndexPrefix + peer_id;

    int play_index = peer_play_map_.value(local_key,-1);
    if(play_index != -1){
        //players[play_index]->StopPlayFile();
        playSource->CloseFile();
        this->update();
        peer_play_map_.insert(local_key,-1);
    }


}

bool VideoWall::Capture(QString peer_id, QString fileName)
{
    int play_index = peer_play_map_.value(peer_id,-1);
    if(play_index != -1){
        //return players[play_index]->Capture(fileName);
        QByteArray data = fileName.toLocal8Bit();
        int ret = playSource->CapPic(data.constData());
        return true;
    }
    qWarning()<<"VideoWall::Capture---id "<<peer_id<<" not found";
    return false;
}

bool VideoWall::OpenSound(QString peerId)
{
    int play_index = peer_play_map_.value(peerId,-1);
    if(play_index != -1){
        //return players[play_index]->OpenSound();
        playSource->OpenSound();
        return true;
    }
    qWarning()<<"VideoWall::OpenSound---id "<<peerId<<" not found";
    return false;
}

bool VideoWall::CloseSound(QString peerId)
{
    int play_index = peer_play_map_.value(peerId,-1);
    if(play_index != -1){
//        return players[play_index]->CloseSound();
        int ret = playSource->CloseSound();
        return true;
    }
    qWarning()<<"VideoWall::CloseSound---id "<<peerId<<" not found";
    return false;
}

bool VideoWall::StartTalk()
{
    if(!talkTimer->isActive()){
        AVService::StartTalk();
        talkTimer->start(15);
        return true;
    }else{
        qDebug("talk already start");
        return false;
    }
}

bool VideoWall::StopTalk()
{
    if(talkTimer->isActive()){
        talkTimer->stop();
        AVService::StopTalk();
        return true;
    }else{
        qDebug("talk not start");
        return false;
    }

}

void VideoWall::OnGetTalkData()
{
    char buf[500];
    int buflen = AVService::GetAudioData(buf);
    if(buflen > 6){
        QByteArray data(&buf[6],buflen-6);
        emit SigTalkData(data);
    }
}



//void VideoWall::setSelectedPlayer(int newSelected)
//{
//    if(m_selectedPlayer != newSelected){
//        this->players[m_selectedPlayer]->setSelected(false);
//        this->m_selectedPlayer = newSelected;
//        this->players[m_selectedPlayer]->setSelected(true);
//    }
//}

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
    int play_index = peer_play_map_.value(peer_id,-1);
    if(play_index != -1){
//        this->players[play_index]->PlayMediaData(data);
        if(!playSource->IsPlaying()){
            HWND playWId = (HWND)this->winId();
            if(playSource->OpenStream(playWId)!=0)
                return;
            playSource->OpenSound();
        }
        playSource->InputStream(data.constData(),data.length());

    }
}

//void VideoWall::createActions()
//{
//    deleteAction = new QAction(tr("&Delete"), this);
//    deleteAction->setShortcut(tr("Delete"));
//    deleteAction->setStatusTip(tr("Delete item from diagram"));
//    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteItem()));

//}

//void VideoWall::createMenus()
//{
//    itemMenu = new QMenu(this);
//    itemMenu->addAction(deleteAction);
//    itemMenu->addSeparator();
//}

//void VideoWall::performDrag(PlayWidget *pw)
//{
//    if(pw == NULL){
//        return;
//    }
//    QByteArray itemData;
//    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
//    dataStream << pw->playIndex();

//    QMimeData *mimeData = new QMimeData;
//    mimeData->setData("application/x-playwidget", itemData);


//    QDrag *drag = new QDrag(this);
//    drag->setMimeData(mimeData);
//    if (drag->exec(Qt::CopyAction | Qt::MoveAction) == Qt::MoveAction) {
//        //drag end
//    }

//    //    if (drag->exec(Qt::MoveAction | Qt::CopyAction, Qt::CopyAction) == Qt::MoveAction){
//    //        qDebug()<<"VideoWall::mousePressEvent-- is Qt::MoveAction";
//    //        //child->close();
//    //    }
//    //    else{
//    //        qDebug()<<"VideoWall::mousePressEvent-- else";

//    //        //child->show();
//    //    }
//}


//void VideoWall::contextMenuEvent(QContextMenuEvent *event)
//{
//    //    qDebug()<<"VideoWall::contextMenuEvent";
//    //    PlayWidget *child = static_cast<PlayWidget*>(this->childAt(event->pos()));
//    //    if (!child){
//    //        qWarning()<<"VideoWall::contextMenuEvent--- no find PlayWidget";
//    //        return;
//    //    }
//    //    int playIndex = child->playIndex();
//    //    //this->setSelectedPlayer(playIndex);

//    //    itemMenu->exec(event->globalPos());

//}

//void VideoWall::paintEvent(QPaintEvent *)
//{
//    //    if(layout_changed_){
//    //        for(int i=0;i<MAX_AVPLAYER;i++){
//    //            //                    if(!this->players[i]->isVisible()){
//    //            //                        this->players[i]->setVisible(true);


//    //            //}
//    //           // this->players[i]->winId();
//    //            // this->players[i]->update();
//    //        }
//    //        //        for(int i=m_layoutList.size();i<MAX_AVPLAYER;i++){
//    //        //            //this->players[i]->winId();
//    //        //            //this->players[i]->setGeometry(-100,-100,0,0);
//    //        //            //this->players[i]->winId();
//    //        //            this->players[i]->setVisible(false);

//    //        //        }
//    //        layout_changed_ = false;
//    //    }


//}

//void VideoWall::mouseMoveEvent(QMouseEvent *event)
//{
//    if (event->buttons() & Qt::LeftButton) {
//        int distance = (event->pos() - startPos).manhattanLength();
//        if (distance >= QApplication::startDragDistance()) {
//            performDrag(this->players[m_selectedPlayer]);
//        }
//    }
//}

//void VideoWall::mousePressEvent(QMouseEvent *event)
//{
//    //qDebug()<<"VideoWall::mousePressEvent";
//    if (event->button() == Qt::LeftButton) {
//        startPos = event->pos();
//    }
//    PlayWidget *child = static_cast<PlayWidget*>(this->childAt(event->pos()));
//    if (!child){
//        //qDebug()<<"VideoWall::mousePressEvent--- no find PlayWidget";
//    }else{
//        int playIndex = child->playIndex();
//        this->setSelectedPlayer(playIndex);
//    }
//    QWidget::mousePressEvent(event);
//}

//void VideoWall::dragEnterEvent(QDragEnterEvent *event)
//{
//    //qDebug()<<"VideoWall::dragEnterEvent";
//    //! [4] //! [5]
//    if (event->mimeData()->hasFormat("application/x-playwidget")) {
//        if (children().contains(event->source())) {
//            event->setDropAction(Qt::MoveAction);
//            event->accept();
//        } else {
//            event->acceptProposedAction();
//        }
//    }  else {
//        event->ignore();
//    }

//}

//void VideoWall::dragMoveEvent(QDragMoveEvent *event)
//{
//    if (event->mimeData()->hasFormat("application/x-playwidget")) {
//        if (children().contains(event->source())) {
//            //qDebug()<<"VideoWall::dragMoveEvent-- children().contains(event->source";
//            event->setDropAction(Qt::MoveAction);
//            event->accept();
//        } else {
//            event->acceptProposedAction();
//        }
//    }  else {
//        event->ignore();
//    }
//}

//void VideoWall::dragLeaveEvent(QDragLeaveEvent *event)
//{
//    //qDebug()<<"VideoWall::dragLeaveEvent";
//    QWidget::dragLeaveEvent(event);
//}

//void VideoWall::dropEvent(QDropEvent *event)
//{
//    if (event->mimeData()->hasFormat("application/x-playwidget")) {
//        const QMimeData *mime = event->mimeData();


//        QByteArray itemData = mime->data("application/x-playwidget");
//        QDataStream dataStream(&itemData, QIODevice::ReadOnly);

//        int moveFrom;
//        //        QPoint offset;
//        dataStream >> moveFrom;

//        PlayWidget *child = static_cast<PlayWidget*>(this->childAt(event->pos()));
//        if (!child){
//            qDebug()<<"VideoWall::dropEvent--- no find PlayWidget";
//            event->ignore();
//            return;
//        }

//        int moveTo = child->playIndex();
//        ExchangePlayWidget(moveFrom,moveTo);


//        //        DragLabel *newLabel = new DragLabel(text, this);
//        //        newLabel->move(event->pos() - offset);
//        //        newLabel->show();
//        //        newLabel->setAttribute(Qt::WA_DeleteOnClose);

//        if (event->source() == this) {
//            event->setDropAction(Qt::MoveAction);
//            event->accept();
//        } else {
//            event->acceptProposedAction();
//        }
//        //! [11] //! [12]
//    } else {
//        event->ignore();
//    }
//}

void VideoWall::keyPressEvent(QKeyEvent *e)
{

    if(e->key() == Qt::Key_Escape){
        showNormalScreenWall();
    }
}
