#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T13:28:30
#
#-------------------------------------------------

QT       -= core gui

TARGET = webrtc_utility
TEMPLATE = lib
CONFIG += staticlib


include (../../webrtc_common.pri)

SOURCES += \
    source/audio_frame_operations.cc \
    source/coder.cc \
    source/file_player_impl.cc \
    source/file_recorder_impl.cc \
    source/frame_scaler.cc \
    source/process_thread_impl.cc \
    source/rtp_dump_impl.cc \
    source/video_coder.cc \
    source/video_frames_queue.cc

HEADERS += \
    source/coder.h \
    source/file_player_impl.h \
    source/file_recorder_impl.h \
    source/frame_scaler.h \
    source/process_thread_impl.h \
    source/rtp_dump_impl.h \
    source/video_coder.h \
    source/video_frames_queue.h \
    interface/process_thread.h


android {
SOURCES += \
    source/helpers_android.cc \

}

OTHER_FILES += \
    source/utility.gypi \
