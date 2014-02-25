#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T13:32:20
#
#-------------------------------------------------

QT       -= core gui

TARGET = media_file
TEMPLATE = lib
CONFIG += staticlib

include (../../webrtc_common.pri)


SOURCES += \
    source/media_file_utility.cc \
    source/media_file_impl.cc \
    source/avi_file.cc

HEADERS += \
    source/media_file_utility.h \
    source/media_file_impl.h \
    source/avi_file.h

OTHER_FILES += \
    source/media_file.gypi
