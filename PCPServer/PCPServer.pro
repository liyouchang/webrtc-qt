#-------------------------------------------------
#
# Project created by QtCreator 2014-02-13T07:59:49
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = PCPServer
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app



Debug:DESTDIR = $$PWD/../Debug
Release:DESTDIR = $$PWD/../Release

INCLUDEPATH     += $$PWD/.. \
    ../third_party/jsoncpp/overrides/include ../third_party/jsoncpp/source/include \

DEFINES += JSONCPP_RELATIVE_PATH

win32 {

Debug:DEFINES +=_DEBUG

Release:DEFINES +=

DEFINES += _UNICODE UNICODE WIN32_LEAN_AND_MEAN

LIBS +=-lwinmm -liphlpapi -lcomsupp -lsecur32 -lws2_32  -lcrypt32 -lAdvapi32 -luser32

} else:mac {
} else {
}


LIBS += -L$$DESTDIR -L$$DESTDIR/lib \
        -ljsoncpp -llibjingle -llibjingle_p2p -llibjingle_app



SOURCES += \
    defaults.cc \
    peer_connection_client.cc \
    ServerConductor.cpp \
    PCPServer.cpp

HEADERS += \
    defaults.h \
    peer_connection_client.h \
    ServerConductor.h
