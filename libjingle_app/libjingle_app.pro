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

INCLUDEPATH     +=  \
    ../third_party/jsoncpp/overrides/include ../third_party/jsoncpp/source/include \



DESTDIR = $$output_dir/libs



SOURCES += \
    ../talk/app/kaerp2p/KaerSession.cpp \
    ../talk/app/kaerp2p/kaersessiondescriptionfactory.cpp \
    ../talk/session/tunnel/tunnelsessionclient.cc \
    ../talk/session/tunnel/pseudotcpchannel.cc \
    ../talk/app/webrtc/jsepicecandidate.cc \
    ../talk/app/webrtc/jsepsessiondescription.cc \
    ../talk/app/webrtc/portallocatorfactory.cc \
    tunnelsdp.cpp \
    ../talk/app/kaerp2p/peertunnel.cpp \
    p2pconductor.cpp \
    streamprocess.cpp \
    ../talk/app/kaerp2p/kaer_session_client.cpp \
    peerterminal.cpp \
    KeMsgProcess.cpp \
    KeMsgProcessContainer.cpp


HEADERS += \
    ../talk/app/kaerp2p/KaerSession.h \
    ../talk/app/kaerp2p/kaersessiondescriptionfactory.h \
    ../talk/session/tunnel/tunnelsessionclient.h \
    ../talk/session/tunnel/pseudotcpchannel.h \
    ../talk/app/webrtc/jsepicecandidate.h \
    ../talk/app/webrtc/jsep.h \
    ../talk/app/webrtc/jsepsessiondescription.h \
    ../talk/app/webrtc/webrtcsdp.h \
    ../talk/app/webrtc/portallocatorfactory.h \
    ../talk/app/kaerp2p/peertunnel.h \
    p2pconductor.h \
    streamprocess.h \
    ../talk/app/kaerp2p/kaer_session_client.h \
    PeerConnectionClinetInterface.h \
    KeMessage.h \
    peerterminal.h \
    KeMsgProcess.h \
    KeMsgProcessContainer.h \
    PeerTerminalInterface.h


