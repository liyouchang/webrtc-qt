#-------------------------------------------------
#
# Project created by QtCreator 2014-02-15T09:35:39
#
#-------------------------------------------------

QT       -= core gui

TARGET = libjingle
TEMPLATE = lib
CONFIG += staticlib

SOURCES += libjingle.cpp

HEADERS += libjingle.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
