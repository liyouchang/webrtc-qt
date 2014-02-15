#-------------------------------------------------
#
# Project created by QtCreator 2014-02-13T09:15:23
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PCPClient
TEMPLATE = app


INCLUDEPATH     += $$PWD/..

include(../talk/talk.pri)


Debug:DESTDIR = $$PWD/../Debug
Release:DESTDIR = $$PWD/../Release

LIBS += -L$$DESTDIR -L$$DESTDIR/lib \
        -ljsoncpp


SOURCES += main.cpp\
        mainwindow.cpp \
    conductor.cc \
    defaults.cc \
    peer_connection_client.cc

HEADERS  += mainwindow.h \
    conductor.h \
    defaults.h \
    main_wnd.h \
    peer_connection_client.h
