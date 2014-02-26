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

INCLUDEPATH += $$webrtc_root/third_party/winsdk_samples/src/Samples/multimedia/directshow/baseclasses

SOURCES += \
        'device_info_impl.cc'\
        'video_capture_factory.cc'\
        'video_capture_impl.cc'\


HEADERS += \
        'device_info_impl.h'\
        'include/video_capture.h'\
        'include/video_capture_defines.h'\
        'include/video_capture_factory.h'\
        'video_capture_config.h'\
        'video_capture_delay.h'\
        'video_capture_impl.h'\


win32 {
SOURCES += \
    windows/device_info_ds.cc \
    windows/device_info_mf.cc \
    windows/help_functions_ds.cc \
    windows/sink_filter_ds.cc \
    windows/video_capture_ds.cc \
    windows/video_capture_factory_windows.cc \
    windows/video_capture_mf.cc


HEADERS += \
    windows/device_info_ds.h \
    windows/device_info_mf.h \
    windows/help_functions_ds.h \
    windows/sink_filter_ds.h \
    windows/video_capture_ds.h \
    windows/video_capture_mf.h


}
linux{


SOURCES += \
    'linux/device_info_linux.cc'\
    'linux/video_capture_linux.cc'\

HEADERS += \
    'linux/device_info_linux.h'\
    'linux/video_capture_linux.h'\

}

OTHER_FILES += \
    video_capture.gypi
