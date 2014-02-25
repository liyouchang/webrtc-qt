#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T14:56:53
#
#-------------------------------------------------

QT       -= core gui

TARGET = PCM16B
TEMPLATE = lib
CONFIG += staticlib

include (../../../../webrtc_common.pri)

INCLUDEPATH += include

SOURCES += \
    pcm16b.c

HEADERS +=

OTHER_FILES += \
    pcm16b.gypi
