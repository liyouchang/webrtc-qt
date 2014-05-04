/********************************************************************************
** Form generated from reading UI file 'TunnelClientUI.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TUNNELCLIENTUI_H
#define UI_TUNNELCLIENTUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <keplayerplugin.h>

QT_BEGIN_NAMESPACE

class Ui_TunnelClientUI
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_2;
    KePlayerPlugin *playPlugin;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout;
    QPushButton *btn_init;
    QLineEdit *edit_peer_id;
    QPushButton *btn_connect;
    QGroupBox *groupBox_ptz;
    QGridLayout *gridLayout;
    QPushButton *ptz_left;
    QPushButton *ptz_right;
    QPushButton *ptz_up;
    QPushButton *ptz_down;
    QGroupBox *groupBox_3;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *clarity_low;
    QPushButton *clarity_normal;
    QPushButton *pushButton_high;
    QPushButton *get_clarity;
    QPushButton *btn_video;
    QPushButton *btn_save_video;
    QPushButton *btn_stop_video;
    QPushButton *btn_disconnect;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *TunnelClientUI)
    {
        if (TunnelClientUI->objectName().isEmpty())
            TunnelClientUI->setObjectName(QStringLiteral("TunnelClientUI"));
        TunnelClientUI->resize(620, 460);
        centralwidget = new QWidget(TunnelClientUI);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        groupBox = new QGroupBox(centralwidget);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(3);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy);
        groupBox->setMinimumSize(QSize(400, 300));
        horizontalLayout_2 = new QHBoxLayout(groupBox);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        playPlugin = new KePlayerPlugin(groupBox);
        playPlugin->setObjectName(QStringLiteral("playPlugin"));

        horizontalLayout_2->addWidget(playPlugin);


        horizontalLayout->addWidget(groupBox);

        groupBox_2 = new QGroupBox(centralwidget);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(1);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(groupBox_2->sizePolicy().hasHeightForWidth());
        groupBox_2->setSizePolicy(sizePolicy1);
        verticalLayout = new QVBoxLayout(groupBox_2);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        btn_init = new QPushButton(groupBox_2);
        btn_init->setObjectName(QStringLiteral("btn_init"));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Maximum);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(btn_init->sizePolicy().hasHeightForWidth());
        btn_init->setSizePolicy(sizePolicy2);

        verticalLayout->addWidget(btn_init);

        edit_peer_id = new QLineEdit(groupBox_2);
        edit_peer_id->setObjectName(QStringLiteral("edit_peer_id"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Maximum);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(edit_peer_id->sizePolicy().hasHeightForWidth());
        edit_peer_id->setSizePolicy(sizePolicy3);

        verticalLayout->addWidget(edit_peer_id);

        btn_connect = new QPushButton(groupBox_2);
        btn_connect->setObjectName(QStringLiteral("btn_connect"));
        sizePolicy2.setHeightForWidth(btn_connect->sizePolicy().hasHeightForWidth());
        btn_connect->setSizePolicy(sizePolicy2);

        verticalLayout->addWidget(btn_connect);

        groupBox_ptz = new QGroupBox(groupBox_2);
        groupBox_ptz->setObjectName(QStringLiteral("groupBox_ptz"));
        QSizePolicy sizePolicy4(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy4.setHorizontalStretch(1);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(groupBox_ptz->sizePolicy().hasHeightForWidth());
        groupBox_ptz->setSizePolicy(sizePolicy4);
        groupBox_ptz->setMinimumSize(QSize(0, 0));
        gridLayout = new QGridLayout(groupBox_ptz);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        ptz_left = new QPushButton(groupBox_ptz);
        ptz_left->setObjectName(QStringLiteral("ptz_left"));

        gridLayout->addWidget(ptz_left, 2, 0, 1, 1);

        ptz_right = new QPushButton(groupBox_ptz);
        ptz_right->setObjectName(QStringLiteral("ptz_right"));

        gridLayout->addWidget(ptz_right, 2, 3, 1, 1);

        ptz_up = new QPushButton(groupBox_ptz);
        ptz_up->setObjectName(QStringLiteral("ptz_up"));

        gridLayout->addWidget(ptz_up, 0, 0, 1, 1);

        ptz_down = new QPushButton(groupBox_ptz);
        ptz_down->setObjectName(QStringLiteral("ptz_down"));

        gridLayout->addWidget(ptz_down, 0, 3, 1, 1);


        verticalLayout->addWidget(groupBox_ptz);

        groupBox_3 = new QGroupBox(groupBox_2);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        QSizePolicy sizePolicy5(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(groupBox_3->sizePolicy().hasHeightForWidth());
        groupBox_3->setSizePolicy(sizePolicy5);
        horizontalLayout_3 = new QHBoxLayout(groupBox_3);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        clarity_low = new QPushButton(groupBox_3);
        clarity_low->setObjectName(QStringLiteral("clarity_low"));
        QSizePolicy sizePolicy6(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy6.setHorizontalStretch(1);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(clarity_low->sizePolicy().hasHeightForWidth());
        clarity_low->setSizePolicy(sizePolicy6);
        clarity_low->setMaximumSize(QSize(30, 16777215));

        horizontalLayout_3->addWidget(clarity_low);

        clarity_normal = new QPushButton(groupBox_3);
        clarity_normal->setObjectName(QStringLiteral("clarity_normal"));
        sizePolicy6.setHeightForWidth(clarity_normal->sizePolicy().hasHeightForWidth());
        clarity_normal->setSizePolicy(sizePolicy6);
        clarity_normal->setMaximumSize(QSize(30, 16777215));

        horizontalLayout_3->addWidget(clarity_normal);

        pushButton_high = new QPushButton(groupBox_3);
        pushButton_high->setObjectName(QStringLiteral("pushButton_high"));
        sizePolicy6.setHeightForWidth(pushButton_high->sizePolicy().hasHeightForWidth());
        pushButton_high->setSizePolicy(sizePolicy6);
        pushButton_high->setMaximumSize(QSize(30, 16777215));

        horizontalLayout_3->addWidget(pushButton_high);

        get_clarity = new QPushButton(groupBox_3);
        get_clarity->setObjectName(QStringLiteral("get_clarity"));
        get_clarity->setMaximumSize(QSize(30, 16777215));

        horizontalLayout_3->addWidget(get_clarity);


        verticalLayout->addWidget(groupBox_3);

        btn_video = new QPushButton(groupBox_2);
        btn_video->setObjectName(QStringLiteral("btn_video"));
        sizePolicy2.setHeightForWidth(btn_video->sizePolicy().hasHeightForWidth());
        btn_video->setSizePolicy(sizePolicy2);

        verticalLayout->addWidget(btn_video);

        btn_save_video = new QPushButton(groupBox_2);
        btn_save_video->setObjectName(QStringLiteral("btn_save_video"));

        verticalLayout->addWidget(btn_save_video);

        btn_stop_video = new QPushButton(groupBox_2);
        btn_stop_video->setObjectName(QStringLiteral("btn_stop_video"));

        verticalLayout->addWidget(btn_stop_video);

        btn_disconnect = new QPushButton(groupBox_2);
        btn_disconnect->setObjectName(QStringLiteral("btn_disconnect"));
        sizePolicy2.setHeightForWidth(btn_disconnect->sizePolicy().hasHeightForWidth());
        btn_disconnect->setSizePolicy(sizePolicy2);

        verticalLayout->addWidget(btn_disconnect);


        horizontalLayout->addWidget(groupBox_2);

        TunnelClientUI->setCentralWidget(centralwidget);
        menubar = new QMenuBar(TunnelClientUI);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 620, 23));
        TunnelClientUI->setMenuBar(menubar);
        statusbar = new QStatusBar(TunnelClientUI);
        statusbar->setObjectName(QStringLiteral("statusbar"));
        TunnelClientUI->setStatusBar(statusbar);

        retranslateUi(TunnelClientUI);

        QMetaObject::connectSlotsByName(TunnelClientUI);
    } // setupUi

    void retranslateUi(QMainWindow *TunnelClientUI)
    {
        TunnelClientUI->setWindowTitle(QApplication::translate("TunnelClientUI", "MainWindow", 0));
        groupBox->setTitle(QApplication::translate("TunnelClientUI", "VideoWall", 0));
        groupBox_2->setTitle(QApplication::translate("TunnelClientUI", "Control", 0));
        btn_init->setText(QApplication::translate("TunnelClientUI", "Init", 0));
        edit_peer_id->setText(QApplication::translate("TunnelClientUI", "1234567", 0));
        btn_connect->setText(QApplication::translate("TunnelClientUI", "ConnectToPeer", 0));
        groupBox_ptz->setTitle(QApplication::translate("TunnelClientUI", "PTZ", 0));
        ptz_left->setText(QApplication::translate("TunnelClientUI", "left", 0));
        ptz_right->setText(QApplication::translate("TunnelClientUI", "right", 0));
        ptz_up->setText(QApplication::translate("TunnelClientUI", "up", 0));
        ptz_down->setText(QApplication::translate("TunnelClientUI", "down", 0));
        groupBox_3->setTitle(QApplication::translate("TunnelClientUI", "Clarity", 0));
        clarity_low->setText(QApplication::translate("TunnelClientUI", "1", 0));
        clarity_normal->setText(QApplication::translate("TunnelClientUI", "2", 0));
        pushButton_high->setText(QApplication::translate("TunnelClientUI", "3", 0));
        get_clarity->setText(QApplication::translate("TunnelClientUI", "101", 0));
        btn_video->setText(QApplication::translate("TunnelClientUI", "Ask Video", 0));
        btn_save_video->setText(QApplication::translate("TunnelClientUI", "save video", 0));
        btn_stop_video->setText(QApplication::translate("TunnelClientUI", "Stop Video", 0));
        btn_disconnect->setText(QApplication::translate("TunnelClientUI", "DisConnectToPeer", 0));
    } // retranslateUi

};

namespace Ui {
    class TunnelClientUI: public Ui_TunnelClientUI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TUNNELCLIENTUI_H
