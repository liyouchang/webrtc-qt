#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T13:46:51
#
#-------------------------------------------------

QT       -= core gui

TARGET = audio_conference_mixer
TEMPLATE = lib
CONFIG += staticlib


include (../../webrtc_common.pri)

SOURCES += \
    source/audio_conference_mixer_impl.cc \
    source/audio_frame_manipulator.cc \
    source/level_indicator.cc \
    source/time_scheduler.cc

HEADERS += \
    source/audio_conference_mixer_impl.h \
    source/audio_frame_manipulator.h \
    source/level_indicator.h \
    source/time_scheduler.h \
    source/memory_pool_posix.h \
    source/memory_pool_win.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}

OTHER_FILES += \
    source/audio_conference_mixer.gypi
