#-------------------------------------------------
#
# Project created by QtCreator 2014-02-22T14:58:08
#
#-------------------------------------------------

QT       -= core gui

TARGET = video_capture_module
TEMPLATE = lib
CONFIG += staticlib

include (../../webrtc_common.pri)


SOURCES += \
        'device_info_impl.cc'\
        'video_capture_factory.cc'\


HEADERS += \
        'device_info_impl.h'\
        'include/video_capture.h'\
        'include/video_capture_defines.h'\
        'include/video_capture_factory.h'\
        'video_capture_config.h'\
        'video_capture_delay.h'\
        'video_capture_impl.cc'\
        'video_capture_impl.h'\



linux{


SOURCES += \
    'linux/device_info_linux.cc'\
    'linux/video_capture_linux.cc'\

HEADERS += \
    'linux/device_info_linux.h'\
    'linux/video_capture_linux.h'\

}
