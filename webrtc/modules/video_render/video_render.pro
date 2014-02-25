#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T09:13:51
#
#-------------------------------------------------

QT       -= core gui

TARGET = video_render
TEMPLATE = lib
CONFIG += staticlib

include (../../webrtc_common.pri)

DEFINES += WEBRTC_INCLUDE_INTERNAL_VIDEO_RENDER

SOURCES += \
    video_render_frames.cc \
    video_render_impl.cc \
    incoming_video_stream.cc \
    linux/video_render_linux_impl.cc \
    linux/video_x11_channel.cc \
    linux/video_x11_render.cc \
    external/video_render_external_impl.cc

HEADERS += \
    video_render_frames.h \
    video_render_impl.h \
    incoming_video_stream.h \
    linux/video_render_linux_impl.h \
    linux/video_x11_channel.h \
    linux/video_x11_render.h \
    external/video_render_external_impl.h

linux {

SOURCES += \
    linux/video_render_linux_impl.cc \
    linux/video_x11_channel.cc \
    linux/video_x11_render.cc

HEADERS += \
    linux/video_render_linux_impl.h \
    linux/video_x11_channel.h \
    linux/video_x11_render.h

}
OTHER_FILES += \
    video_render.gypi
