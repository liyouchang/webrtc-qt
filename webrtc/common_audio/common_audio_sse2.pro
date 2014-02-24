#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T09:55:58
#
#-------------------------------------------------

QT       -= core gui

TARGET = common_audio_sse2
TEMPLATE = lib
CONFIG += staticlib


include (../webrtc_common.pri)

QMAKE_CFLAGS += -msse2

SOURCES += \
            'resampler/sinc_resampler_sse.cc'

HEADERS +=
