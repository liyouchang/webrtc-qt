#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T13:58:08
#
#-------------------------------------------------

QT       -= core gui

TARGET = G722
TEMPLATE = lib
CONFIG += staticlib


include (../../../../webrtc_common.pri)

INCLUDEPATH += include

SOURCES += \
    g722_decode.c \
    g722_encode.c \
    g722_interface.c

HEADERS += \
    g722_enc_dec.h

OTHER_FILES += \
    g722.gypi
