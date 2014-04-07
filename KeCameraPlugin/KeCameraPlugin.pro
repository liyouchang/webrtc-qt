#-------------------------------------------------
#
# Project created by QtCreator 2013-09-26T10:10:52
#
#-------------------------------------------------
QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT += network

Debug:DESTDIR = $$PWD/../Debug
Release:DESTDIR = $$PWD/../Release
TARGET = CameraPlugin
#TEMPLATE = lib


DEFINES += KECAMERAPLUGIN_LIBRARY

SOURCES += CameraPlugin.cpp \
    mythread.cpp

HEADERS += CameraPlugin.h \
    mythread.h


win32 {
   RC_FILE = KeCameraPlugin.rc
}


include(npapi/qtbrowserplugin.pri)
include(../KeClientCommon/keClientCommon.pri)
