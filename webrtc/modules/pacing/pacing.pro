#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T15:23:13
#
#-------------------------------------------------

QT       -= core gui

TARGET = paced_sender
TEMPLATE = lib
CONFIG += staticlib

include (../../webrtc_common.pri)

SOURCES += \
    paced_sender.cc

HEADERS +=
unix {
    target.path = /usr/lib
    INSTALLS += target
}

OTHER_FILES += \
    pacing.gypi
