#ifndef PLAYPANNEL_H
#define PLAYPANNEL_H

#include<QtWidgets>
#include"AVService.h"

#define MAX_AVPLAYER 64
class PlayWidget;

class VideoWall: public QWidget
{
    Q_OBJECT

public:
    explicit VideoWall(QWidget *parent = 0);
    ~VideoWall();

    enum ScreenDivisionType{
        ScreenDivision_None = 0,
        ScreenDivision_One = 1,
        ScreenDivision_Four = 4 ,
        ScreenDivision_Six  = 6,
        ScreenDivision_Eight  = 8,
        ScreenDivision_Nine = 9,
        ScreenDivision_Ten1 = 10,
        ScreenDivision_Ten2 = 11,
        ScreenDivision_Thirteen1 = 12,
        ScreenDivision_Thirteen2 = 13,
        ScreenDivision_Fourteen = 14,
        ScreenDivision_Sixteen = 16,
        ScreenDivision_Seventeen = 17,
        ScreenDivision_Nineteen = 19,
        ScreenDivision_Twentytwo = 22,
        ScreenDivision_Twentyfive = 25,
        ScreenDivision_Thirtytwo = 32,
        ScreenDivision_Sixtyfour = 64,
        ScreenDivision_Max
    };

    void SetDivision(ScreenDivisionType divType);
    void SetDivision(int num);
    void PlayLocalFile();

public slots:
    void setSelectedPlayer(int newSelected);
    void deleteItem();
    void showfullScreenWall();
    void showNormalScreenWall();
    void OnRecvMediaData(QString peer_id, QByteArray data);

protected:
    AVService * playSource;

    QGridLayout *mainLayout;
    PlayWidget * players[MAX_AVPLAYER];
    int selectedPlayer;
    int oldDivType;

    QMenu *itemMenu;


    QAction *deleteAction;
private:
    void createActions();
    void createMenus();
protected:
    void paintEvent(QPaintEvent *);

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *e);
};


#endif // PLAYPANNEL_H
