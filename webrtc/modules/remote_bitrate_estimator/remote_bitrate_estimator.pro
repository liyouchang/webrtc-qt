#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T15:26:45
#
#-------------------------------------------------

QT       -= core gui

TARGET = remote_bitrate_estimator
TEMPLATE = lib
CONFIG += staticlib

include (../../webrtc_common.pri)

SOURCES += \
    rate_statistics.cc \
    rtp_to_ntp.cc \
    overuse_detector.cc \
    remote_bitrate_estimator_single_stream.cc \
    remote_rate_control.cc

HEADERS += \
    rate_statistics.h \
    overuse_detector.h \
    remote_rate_control.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}

OTHER_FILES += \
    remote_bitrate_estimator.gypi \
    remote_bitrate_estimator_components.gyp
