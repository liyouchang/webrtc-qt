#-------------------------------------------------
#
# Project created by QtCreator 2014-02-19T16:23:40
#
#-------------------------------------------------

QT       -= core gui

TARGET = jingle_peerconnection
TEMPLATE = lib
CONFIG += staticlib

DESTDIR = $$PWD/../libs


include (talk_common.pri)


HEADERS += \
        'app/webrtc/audiotrack.h'\
        'app/webrtc/audiotrackrenderer.h'\
        'app/webrtc/datachannel.h'\
        'app/webrtc/datachannelinterface.h'\
        'app/webrtc/dtmfsender.h'\
        'app/webrtc/dtmfsenderinterface.h'\
        'app/webrtc/fakeportallocatorfactory.h'\
        'app/webrtc/jsep.h'\
        'app/webrtc/jsepicecandidate.h'\
        'app/webrtc/jsepsessiondescription.h'\
        'app/webrtc/localaudiosource.h'\
        'app/webrtc/mediaconstraintsinterface.h'\
        'app/webrtc/mediastream.h'\
        'app/webrtc/mediastreamhandler.h'\
        'app/webrtc/mediastreaminterface.h'\
        'app/webrtc/mediastreamprovider.h'\
        'app/webrtc/mediastreamproxy.h'\
        'app/webrtc/mediastreamsignaling.h'\
        'app/webrtc/mediastreamtrack.h'\
        'app/webrtc/mediastreamtrackproxy.h'\
        'app/webrtc/notifier.h'\
        'app/webrtc/peerconnection.h'\
        'app/webrtc/peerconnectionfactory.h'\
        'app/webrtc/peerconnectioninterface.h'\
        'app/webrtc/peerconnectionproxy.h'\
        'app/webrtc/portallocatorfactory.h'\
        'app/webrtc/proxy.h'\
        'app/webrtc/remoteaudiosource.h'\
        'app/webrtc/remotevideocapturer.h'\
        'app/webrtc/sctputils.h'\
        'app/webrtc/statscollector.h'\
        'app/webrtc/statstypes.h'\
        'app/webrtc/streamcollection.h'\
        'app/webrtc/videosource.h'\
        'app/webrtc/videosourceinterface.h'\
        'app/webrtc/videosourceproxy.h'\
        'app/webrtc/videotrack.h'\
        'app/webrtc/videotrackrenderers.h'\
        'app/webrtc/webrtcsdp.h'\
        'app/webrtc/webrtcsession.h'\
        'app/webrtc/webrtcsessiondescriptionfactory.h'\



SOURCES += \
        'app/webrtc/audiotrack.cc'\
        'app/webrtc/audiotrackrenderer.cc'\
        'app/webrtc/datachannel.cc'\
        'app/webrtc/dtmfsender.cc'\
        'app/webrtc/jsepicecandidate.cc'\
        'app/webrtc/jsepsessiondescription.cc'\
        'app/webrtc/localaudiosource.cc'\
        'app/webrtc/mediaconstraintsinterface.cc'\
        'app/webrtc/mediastream.cc'\
        'app/webrtc/mediastreamhandler.cc'\
        'app/webrtc/mediastreamsignaling.cc'\
        'app/webrtc/peerconnection.cc'\
        'app/webrtc/peerconnectionfactory.cc'\
        'app/webrtc/portallocatorfactory.cc'\
        'app/webrtc/remoteaudiosource.cc'\
        'app/webrtc/remotevideocapturer.cc'\
        'app/webrtc/sctputils.cc'\
        'app/webrtc/statscollector.cc'\
        'app/webrtc/videosource.cc'\
        'app/webrtc/videotrack.cc'\
        'app/webrtc/videotrackrenderers.cc'\
        'app/webrtc/webrtcsdp.cc'\
        'app/webrtc/webrtcsession.cc'\
        'app/webrtc/webrtcsessiondescriptionfactory.cc'\

