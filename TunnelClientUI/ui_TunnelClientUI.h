/********************************************************************************
** Form generated from reading UI file 'TunnelClientUI.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TUNNELCLIENTUI_H
#define UI_TUNNELCLIENTUI_H

#include <KePlayView/VideoWall.h>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
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

QT_BEGIN_NAMESPACE

class Ui_TunnelClientUI
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_2;
    VideoWall *videoWall;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout;
    QPushButton *btn_init;
    QLineEdit *edit_peer_id;
    QPushButton *btn_connect;
    QPushButton *btn_video;
    QPushButton *btn_disconnect;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *TunnelClientUI)
    {
        if (TunnelClientUI->objectName().isEmpty())
            TunnelClientUI->setObjectName(QStringLiteral("TunnelClientUI"));
        TunnelClientUI->resize(640, 480);
        centralwidget = new QWidget(TunnelClientUI);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        groupBox = new QGroupBox(centralwidget);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(3);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy);
        horizontalLayout_2 = new QHBoxLayout(groupBox);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        videoWall = new VideoWall(groupBox);
        videoWall->setObjectName(QStringLiteral("videoWall"));

        horizontalLayout_2->addWidget(videoWall);


        horizontalLayout->addWidget(groupBox);

        groupBox_2 = new QGroupBox(centralwidget);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
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

        btn_video = new QPushButton(groupBox_2);
        btn_video->setObjectName(QStringLiteral("btn_video"));
        sizePolicy2.setHeightForWidth(btn_video->sizePolicy().hasHeightForWidth());
        btn_video->setSizePolicy(sizePolicy2);

        verticalLayout->addWidget(btn_video);

        btn_disconnect = new QPushButton(groupBox_2);
        btn_disconnect->setObjectName(QStringLiteral("btn_disconnect"));
        sizePolicy2.setHeightForWidth(btn_disconnect->sizePolicy().hasHeightForWidth());
        btn_disconnect->setSizePolicy(sizePolicy2);

        verticalLayout->addWidget(btn_disconnect);


        horizontalLayout->addWidget(groupBox_2);

        TunnelClientUI->setCentralWidget(centralwidget);
        menubar = new QMenuBar(TunnelClientUI);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 640, 23));
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
        btn_connect->setText(QApplication::translate("TunnelClientUI", "ConnectToPeer", 0));
        btn_video->setText(QApplication::translate("TunnelClientUI", "Ask Video", 0));
        btn_disconnect->setText(QApplication::translate("TunnelClientUI", "DisConnectToPeer", 0));
    } // retranslateUi

};

namespace Ui {
    class TunnelClientUI: public Ui_TunnelClientUI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TUNNELCLIENTUI_H
