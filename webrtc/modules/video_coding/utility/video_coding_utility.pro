#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T15:36:37
#
#-------------------------------------------------

QT       -= core gui

TARGET = video_coding_utility
TEMPLATE = lib
CONFIG += staticlib

include (../../../webrtc_common.pri)

SOURCES += \
    frame_dropper.cc \
    exp_filter.cc

HEADERS +=


OTHER_FILES += \
    video_coding_utility.gyp
