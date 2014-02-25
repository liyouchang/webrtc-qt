#-------------------------------------------------
#
# Project created by QtCreator 2014-02-25T09:31:05
#
#-------------------------------------------------

QT       -= core gui

TARGET = webrtc_i420
TEMPLATE = lib
CONFIG += staticlib

include (../../../../webrtc_common.pri)

SOURCES += \
    main/source/i420.cc

HEADERS +=

OTHER_FILES += \
    main/source/i420.gypi
