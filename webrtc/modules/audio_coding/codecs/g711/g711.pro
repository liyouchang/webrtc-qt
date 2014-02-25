#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T14:15:10
#
#-------------------------------------------------

QT       -= core gui

TARGET = G711
TEMPLATE = lib
CONFIG += staticlib

include (../../../../webrtc_common.pri)

INCLUDEPATH += include

SOURCES += \
    g711.c \
    g711_interface.c

HEADERS += g711.h


OTHER_FILES += \
    g711.gypi
