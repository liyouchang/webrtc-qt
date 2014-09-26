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

INCLUDEPATH += ../third_party/mp4v2/include

#target.path = /libs
#INSTALLS += target
#target_lib.files = $$OUT_PWD/libjingle_app.a
#target_lib.path = $$output_dir/libs
#INSTALLS += target_lib
#message($$OUT_PWD/$$TARGET)


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
    defaults.cc \
    jsonconfig.cpp \
    ketunnelclient.cpp \
    ketunnelcamera.cpp \
    recorderavi.cpp \
    ke08recorder.cpp \
    kelocalclient.cpp \
    ../talk/app/kaerp2p/udpstreamchannel.cpp \
    ../talk/session/tunnel/streamchannelinterface.cpp \
    mp4encoder.cpp \
    recordermp4.cpp


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
    PeerTerminalInterface.h \
    defaults.h \
    jsonconfig.h \
    ketunnelclient.h \
    ketunnelcamera.h \
    recorderavi.h \
    recordinterface.h \
    ke08recorder.h \
    kelocalclient.h \
    ../talk/app/kaerp2p/udpstreamchannel.h \
    ../talk/session/tunnel/streamchannelinterface.h \
    mp4encoder.h \
    recordermp4.h

OTHER_FILES += \
    ClientCameraMsg.json


