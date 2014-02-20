/********************************************************************************
** Form generated from reading UI file 'peerclientwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PEERCLIENTWINDOW_H
#define UI_PEERCLIENTWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QWidget>
#include "StreamingUIWidget.h"

QT_BEGIN_NAMESPACE

class Ui_PeerClientWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout_2;
    QStackedWidget *stackedWidget;
    QWidget *page_login;
    QHBoxLayout *horizontalLayout_4;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *edit_IP;
    QLabel *label_2;
    QLineEdit *edit_Port;
    QPushButton *btn_connect;
    QWidget *page_peerList;
    QHBoxLayout *horizontalLayout_3;
    QTreeWidget *peerList;
    QWidget *page_stream;
    QHBoxLayout *horizontalLayout_5;
    StreamingUIWidget *streamShow;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *PeerClientWindow)
    {
        if (PeerClientWindow->objectName().isEmpty())
            PeerClientWindow->setObjectName(QStringLiteral("PeerClientWindow"));
        PeerClientWindow->resize(640, 480);
        QPalette palette;
        QBrush brush(QColor(255, 255, 255, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Base, brush);
        QBrush brush1(QColor(202, 202, 202, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Window, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Window, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Window, brush1);
        PeerClientWindow->setPalette(palette);
        centralwidget = new QWidget(PeerClientWindow);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        horizontalLayout_2 = new QHBoxLayout(centralwidget);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName(QStringLiteral("stackedWidget"));
        stackedWidget->setAutoFillBackground(false);
        page_login = new QWidget();
        page_login->setObjectName(QStringLiteral("page_login"));
        horizontalLayout_4 = new QHBoxLayout(page_login);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        groupBox = new QGroupBox(page_login);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        horizontalLayout = new QHBoxLayout(groupBox);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);

        edit_IP = new QLineEdit(groupBox);
        edit_IP->setObjectName(QStringLiteral("edit_IP"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(3);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(edit_IP->sizePolicy().hasHeightForWidth());
        edit_IP->setSizePolicy(sizePolicy);

        horizontalLayout->addWidget(edit_IP);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));

        horizontalLayout->addWidget(label_2);

        edit_Port = new QLineEdit(groupBox);
        edit_Port->setObjectName(QStringLiteral("edit_Port"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(1);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(edit_Port->sizePolicy().hasHeightForWidth());
        edit_Port->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(edit_Port);

        btn_connect = new QPushButton(groupBox);
        btn_connect->setObjectName(QStringLiteral("btn_connect"));

        horizontalLayout->addWidget(btn_connect);


        horizontalLayout_4->addWidget(groupBox);

        stackedWidget->addWidget(page_login);
        page_peerList = new QWidget();
        page_peerList->setObjectName(QStringLiteral("page_peerList"));
        horizontalLayout_3 = new QHBoxLayout(page_peerList);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        peerList = new QTreeWidget(page_peerList);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QStringLiteral("1"));
        peerList->setHeaderItem(__qtreewidgetitem);
        peerList->setObjectName(QStringLiteral("peerList"));
        peerList->setHeaderHidden(true);

        horizontalLayout_3->addWidget(peerList);

        stackedWidget->addWidget(page_peerList);
        page_stream = new QWidget();
        page_stream->setObjectName(QStringLiteral("page_stream"));
        horizontalLayout_5 = new QHBoxLayout(page_stream);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        streamShow = new StreamingUIWidget(page_stream);
        streamShow->setObjectName(QStringLiteral("streamShow"));

        horizontalLayout_5->addWidget(streamShow);

        stackedWidget->addWidget(page_stream);

        horizontalLayout_2->addWidget(stackedWidget);

        PeerClientWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(PeerClientWindow);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 640, 23));
        PeerClientWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(PeerClientWindow);
        statusbar->setObjectName(QStringLiteral("statusbar"));
        PeerClientWindow->setStatusBar(statusbar);

        retranslateUi(PeerClientWindow);

        stackedWidget->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(PeerClientWindow);
    } // setupUi

    void retranslateUi(QMainWindow *PeerClientWindow)
    {
        PeerClientWindow->setWindowTitle(QApplication::translate("PeerClientWindow", "MainWindow", 0));
        groupBox->setTitle(QApplication::translate("PeerClientWindow", "Connect to Server", 0));
        label->setText(QApplication::translate("PeerClientWindow", "Server", 0));
        edit_IP->setText(QApplication::translate("PeerClientWindow", "219.146.201.106", 0));
        label_2->setText(QApplication::translate("PeerClientWindow", ":", 0));
        edit_Port->setText(QApplication::translate("PeerClientWindow", "8888", 0));
        btn_connect->setText(QApplication::translate("PeerClientWindow", "connect", 0));
    } // retranslateUi

};

namespace Ui {
    class PeerClientWindow: public Ui_PeerClientWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PEERCLIENTWINDOW_H
