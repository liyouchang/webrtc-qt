#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T14:04:19
#
#-------------------------------------------------

QT       -= core gui

TARGET = CNG
TEMPLATE = lib
CONFIG += staticlib

include (../../../../webrtc_common.pri)


INCLUDEPATH += include $$webrtc_root/webrtc/common_audio/signal_processing/include

SOURCES += \
    cng_helpfuns.c \
    webrtc_cng.c

HEADERS += \
    cng_helpfuns.h

OTHER_FILES += \
    cng.gypi
