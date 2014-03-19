#-------------------------------------------------
#
# Project created by QtCreator 2014-03-18T17:33:15
#
#-------------------------------------------------

QT       -= core gui

TARGET = ZmqPeerTeminal
TEMPLATE = lib



include (../talk/talk_common.pri)

DESTDIR = $$output_dir

INCLUDEPATH     +=  \
    ../third_party/jsoncpp/overrides/include ../third_party/jsoncpp/source/include \
    ../third_party/cppzmq/include


SOURCES += zmqpeerteminal.cpp \
    ../zmqclient/asyndealer.cpp \
    ../zmqclient/peerconnectionclientdealer.cpp \
    ../zmqclient/peerterminal.cpp

HEADERS += zmqpeerteminal.h \
    ../zmqclient/asyndealer.h \
    ../zmqclient/peerconnectionclientdealer.h \
    ../zmqclient/peerterminal.h
