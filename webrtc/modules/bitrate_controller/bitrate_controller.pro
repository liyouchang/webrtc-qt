#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T15:52:40
#
#-------------------------------------------------

QT       -= core gui

TARGET = bitrate_controller
TEMPLATE = lib
CONFIG += staticlib

include (../../webrtc_common.pri)

SOURCES += \
    bitrate_controller_impl.cc \
    send_side_bandwidth_estimation.cc

HEADERS += \
    bitrate_controller_impl.h \
    send_side_bandwidth_estimation.h


OTHER_FILES += \
    bitrate_controller.gypi
