#ifndef PLAYPANNEL_H
#define PLAYPANNEL_H

#include <QWidget>
#include <QVector>
#include <QMap>

#define MAX_AVPLAYER 1
class AVService;
//class PlayWidget;



QT_BEGIN_NAMESPACE
class QDragEnterEvent;
class QDropEvent;
class QGridLayout;
class QVBoxLayout;
class QMenu;
QT_END_NAMESPACE


class VideoWall: public QWidget
{
    Q_OBJECT

public:
    explicit VideoWall(QWidget *parent = 0);
    ~VideoWall();

//    enum ScreenDivisionType{
//        ScreenDivision_None = 0,
//        ScreenDivision_One = 1,
//        ScreenDivision_Four = 4 ,
//        ScreenDivision_Six  = 6,
//        ScreenDivision_Eight  = 8,
//        ScreenDivision_Nine = 9,
//        ScreenDivision_Ten1 = 10,
//        ScreenDivision_Ten2 = 11,
//        ScreenDivision_Thirteen1 = 12,
//        ScreenDivision_Thirteen2 = 13,
//        ScreenDivision_Fourteen = 14,
//        ScreenDivision_Sixteen = 16,
//        ScreenDivision_Seventeen = 17,
//        ScreenDivision_Nineteen = 19,
//        ScreenDivision_Twentytwo = 22,
//        ScreenDivision_Twentyfive = 25,
//        ScreenDivision_Thirtytwo = 32,
//        ScreenDivision_Sixtyfour = 64,
//        ScreenDivision_Max
//    };

//    void SetDivision(ScreenDivisionType divType);
//    void SetDivision(int num);
//    void BuildLayout();

//    void ExchangePlayWidget(int from,int to);
    int SetPeerPlay(QString peer_id);
    //player_num == -1 for the current player
    QString GetPeerPlay(int player_num = -1);
    void StopPeerPlay(QString peer_id);
    bool SetLocalPlayFileSize(QString peer_id,int size);
    void StopFilePlay(QString peer_id);
    bool Capture(QString peer_id,QString fileName);

    bool OpenSound(QString peerId);
    bool CloseSound(QString peerId);

    QTimer *talkTimer;
    bool StartTalk();
    bool StopTalk();
signals:
    void SigNeedStopPeerPlay(QString peer_id);
    void SigTalkData(QByteArray data);
public slots:
    void OnGetTalkData();

    //void setSelectedPlayer(int newSelected);
    void deleteItem();
    void showfullScreenWall();
    void showNormalScreenWall();
    void OnRecvMediaData(QString peer_id, QByteArray data);

    void PlayLocalFile(QString peer_id, QString file_name, int file_size);

private:
    AVService * playSource;
//    QGridLayout *mainLayout;
//    QVBoxLayout *hideLayout;

//    PlayWidget * players[MAX_AVPLAYER];
    int m_selectedPlayer;
//    int m_divType;
//    QVector<int> m_layoutList;
    //某个id的播放号和播放屏幕编号的对应，对于实时视频使用peer_id作为key，
    //对于录像回放，使用kLocalIndexPrefix+peer_id作为key
    QMap<QString,int> peer_play_map_;

//    QMenu *itemMenu;
//    QPoint startPos;
//    QAction *deleteAction;
    bool layout_changed_;
private:
//    void createActions();
//    void createMenus();
//    void performDrag(PlayWidget *pw);

    // QWidget interface
protected:
//    void mousePressEvent(QMouseEvent *event);
//    void mouseMoveEvent(QMouseEvent *);
//    void dragEnterEvent(QDragEnterEvent *event);
//    void dragMoveEvent(QDragMoveEvent *event);
//    void dragLeaveEvent(QDragLeaveEvent *event);
//    void dropEvent(QDropEvent *);
    void keyPressEvent(QKeyEvent *e);
//    void contextMenuEvent(QContextMenuEvent *event);
    //void paintEvent(QPaintEvent *);

};


#endif // PLAYPANNEL_H
