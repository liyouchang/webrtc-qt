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

DESTDIR = $$PWD/../libs

INCLUDEPATH     += ..


win32 {

Debug:DEFINES +=_DEBUG

Release:DEFINES +=

DEFINES += _UNICODE UNICODE


} else:mac {
} else {
    DEFINES += POSIX LOGGING=1
}


SOURCES += \
    ../talk/media/base/codec.cc \
    ../talk/media/sctp/sctputils.cc \
    ../talk/app/webrtc/jsepicecandidate.cc \
    ../talk/app/webrtc/jsepsessiondescription.cc \
    ../talk/app/webrtc/webrtcsdp.cc \
    ../talk/app/kaerp2p/KaerSession.cpp \
    ../talk/app/kaerp2p/kaer_session_client.cpp \
    ../talk/session/tunnel/pseudotcpchannel.cc \
    ../talk/session/tunnel/tunnelsessionclient.cc \
    ../talk/app/kaerp2p/kaersessiondescriptionfactory.cpp


HEADERS += \
    ../talk/media/base/codec.h \
    ../talk/media/sctp/sctpdataengine.h \
    ../talk/media/sctp/sctputils.h \
    ../talk/app/webrtc/webrtcsdp.h \
    ../talk/app/kaerp2p/KaerSession.h \
    ../talk/app/kaerp2p/kaer_session_client.h \
    ../talk/session/tunnel/pseudotcpchannel.h \
    ../talk/session/tunnel/tunnelsessionclient.h \
    ../talk/app/kaerp2p/kaersessiondescriptionfactory.h
