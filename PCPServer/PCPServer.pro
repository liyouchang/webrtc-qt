#-------------------------------------------------
#
# Project created by QtCreator 2014-02-13T07:59:49
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = PCPServer
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


INCLUDEPATH     += $$PWD/..

include(../talk/talk.pri)


Debug:DESTDIR = $$PWD/../Debug
Release:DESTDIR = $$PWD/../Release

LIBS += -L$$DESTDIR -L$$DESTDIR/lib \
        -ljsoncpp


SOURCES += \
    defaults.cc \
    peer_connection_client.cc \
    ServerConductor.cpp \
    PCPServer.cpp

HEADERS += \
    defaults.h \
    peer_connection_client.h \
    ServerConductor.h
