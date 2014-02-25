#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T16:11:49
#
#-------------------------------------------------

QT       -= core gui

TARGET = webrtc_vp8
TEMPLATE = lib
CONFIG += staticlib

include (../../../../webrtc_common.pri)


INCLUDEPATH += $$webrtc_root/third_party/libvpx/source/libvpx

SOURCES += \
    reference_picture_selection.cc \
    default_temporal_layers.cc \
    vp8_impl.cc \
    realtime_temporal_layers.cc

HEADERS += \
    reference_picture_selection.h \
    default_temporal_layers.h \
    vp8_impl.h \
    temporal_layers.h

OTHER_FILES += \
    vp8.gyp
