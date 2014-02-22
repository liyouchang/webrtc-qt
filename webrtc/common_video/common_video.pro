#-------------------------------------------------
#
# Project created by QtCreator 2014-02-22T14:39:33
#
#-------------------------------------------------

QT       -= core gui

TARGET = common_video
TEMPLATE = lib
CONFIG += staticlib



include (../webrtc_common.pri)

INCLUDEPATH += $$PWD/../../third_party/libyuv/include

SOURCES += \
        'i420_video_frame.cc'\
        'libyuv/webrtc_libyuv.cc'\
        'libyuv/scaler.cc'\
        'plane.cc'\
        'texture_video_frame.cc'


HEADERS += \
        'interface/i420_video_frame.h'\
        'interface/native_handle.h'\
        'interface/texture_video_frame.h'\
        'libyuv/include/webrtc_libyuv.h'\
        'plane.h'\
        'libyuv/include/scaler.h'\



