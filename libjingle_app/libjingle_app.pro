#-------------------------------------------------
#
# Project created by QtCreator 2014-02-15T09:52:46
#
#-------------------------------------------------

QT       -= core gui

TARGET = jingle_app
TEMPLATE = lib
CONFIG += staticlib

CONFIG -= app_bundle
CONFIG -= qt


include (../talk/talk_common.pri)

DESTDIR = $$output_dir/libs



SOURCES += \
    ../talk/app/kaerp2p/KaerSession.cpp \
    ../talk/app/kaerp2p/kaer_session_client.cpp \
    ../talk/app/kaerp2p/kaersessiondescriptionfactory.cpp


HEADERS += \
    ../talk/app/kaerp2p/KaerSession.h \
    ../talk/app/kaerp2p/kaer_session_client.h \
    ../talk/app/kaerp2p/kaersessiondescriptionfactory.h
