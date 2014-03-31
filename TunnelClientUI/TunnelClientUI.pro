#-------------------------------------------------
#
# Project created by QtCreator 2014-03-19T10:18:23
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TunnelClientUI
TEMPLATE = app

include (../talk/talk_common.pri)

DESTDIR = $$output_dir

INCLUDEPATH     +=  $$PWD\
    ../third_party/jsoncpp/overrides/include ../third_party/jsoncpp/source/include \
    ../third_party/cppzmq/include


win32 {


#for zmqhelper
DEFINES += __WINDOWS__ ZMQ_STATIC

LIBS +=-lwinmm -liphlpapi -lcomsupp -lsecur32 -lws2_32  -lcrypt32 -lAdvapi32 -luser32
LIBS += -lStrmiids -lshell32 -lrpcrt4

    LIBS += -L$$output_dir/libs
    LIBS += -ljsoncpp -lcppzmq
    LIBS += -ljingle_app  -ljingle_p2p  -ljingle



} else {
    #QMAKE_CXXFLAGS += -std=c++11

    LIBS += -L$$output_dir/libs

    LIBS +=   -ljingle_app  -ljingle_p2p  -ljingle
    LIBS += -ljsoncpp  -lcppzmq

    LIBS += -pthread -ldl

    #PRE_TARGETDEPS += $$DESTDIR/libjsoncpp.a $$DESTDIR/libjingle_p2p.a $$DESTDIR/libjingle.a $$DESTDIR/libjingle_app.a

}

include(./KePlayView/KePlayView.pri)

SOURCES +=\
    TunnelClientUI.cpp \
    ../zmqclient/asyndealer.cpp \
    ../zmqclient/peerconnectionclientdealer.cpp \
    KeQtTunnelClient.cpp

HEADERS  += \
    TunnelClientUI.h \
    ../zmqclient/asyndealer.h \
    ../zmqclient/peerconnectionclientdealer.h \
    KeQtTunnelClient.h

FORMS += \
    TunnelClientUI.ui
