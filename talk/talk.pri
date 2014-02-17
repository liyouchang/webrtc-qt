#-------------------------------------------------
#
# Project created by QtCreator 2013-12-20T11:36:54
#
#-------------------------------------------------

INCLUDEPATH     += $$PWD/.. \
    ../third_party/jsoncpp/overrides/include ../third_party/jsoncpp/source/include \

DEFINES += JSONCPP_RELATIVE_PATH


win32 {

Debug:DEFINES +=_DEBUG

Release:DEFINES +=

DEFINES += _UNICODE UNICODE WIN32_LEAN_AND_MEAN

LIBS +=-lwinmm -liphlpapi -lcomsupp -lsecur32 -lws2_32  -lcrypt32 -lAdvapi32 -luser32

SOURCES += \
    $$PWD/base/win32securityerrors.cc \
    $$PWD/base/win32socketinit.cc \
    $$PWD/base/winping.cc \
    $$PWD/base/win32socketserver.cc \
    $$PWD/base/win32window.cc \
    $$PWD/base/schanneladapter.cc \
    $$PWD/base/win32socketserver.h \
    $$PWD/base/win32window.h \
    $$PWD/base/win32.cc \


HEADERS += \
    $$PWD/base/winping.h \
    $$PWD/base/win32socketinit.h \
    $$PWD/base/win32.h \
    $$PWD/base/schanneladapter.h \


} else:mac {
} else {
}


SOURCES += \
    $$PWD/base/socketaddress.cc \
    $$PWD/base/ipaddress.cc \
    $$PWD/base/nethelpers.cc \
    $$PWD/base/signalthread.cc \
    $$PWD/base/messagehandler.cc \
    $$PWD/base/messagequeue.cc \
    $$PWD/base/thread.cc \
    $$PWD/base/timeutils.cc \
    $$PWD/base/physicalsocketserver.cc \
    $$PWD/base/common.cc \
    $$PWD/base/asyncsocket.cc \
    $$PWD/base/logging.cc \
    $$PWD/base/stream.cc \
    $$PWD/base/stringencode.cc \
    $$PWD/base/network.cc \
    $$PWD/base/virtualsocketserver.cc \
    $$PWD/base/asyncudpsocket.cc \
    $$PWD/base/helpers.cc \
    $$PWD/base/ssladapter.cc \
    $$PWD/base/socketadapters.cc \
    $$PWD/base/socketaddresspair.cc \
    $$PWD/base/messagedigest.cc \
    $$PWD/base/bytebuffer.cc \
    $$PWD/base/crc32.cc \
    $$PWD/base/base64.cc \
    $$PWD/base/ratetracker.cc \
    $$PWD/base/md5.cc \
    $$PWD/base/sha1.cc \
    $$PWD/base/httpcommon.cc \
    $$PWD/base/firewallsocketserver.cc \
    $$PWD/base/natsocketfactory.cc \
    $$PWD/base/natserver.cc \
    $$PWD/base/proxyserver.cc \
    $$PWD/base/nattypes.cc \
    $$PWD/base/sslstreamadapter.cc \
    $$PWD/base/json.cc \
    $$PWD/base/stringutils.cc \
    $$PWD/base/asynctcpsocket.cc \
    $$PWD/p2p/base/constants.cc \
    $$PWD/p2p/base/basicpacketsocketfactory.cc \
    $$PWD/p2p/base/port.cc \
    $$PWD/p2p/base/stun.cc \
    $$PWD/p2p/base/stunport.cc \
    $$PWD/p2p/base/stunrequest.cc \
    $$PWD/p2p/base/stunserver.cc \
    $$PWD/p2p/base/asyncstuntcpsocket.cc \
    $$PWD/p2p/base/portallocator.cc \
    $$PWD/p2p/base/relayserver.cc \
    $$PWD/p2p/base/p2ptransportchannel.cc \
    $$PWD/p2p/base/portallocatorsessionproxy.cc \
    $$PWD/p2p/base/tcpport.cc \
    $$PWD/p2p/base/relayport.cc \
    $$PWD/p2p/base/turnport.cc \
    $$PWD/p2p/base/transportchannel.cc \
    $$PWD/p2p/base/portproxy.cc \
    $$PWD/p2p/base/pseudotcp.cc \
    $$PWD/p2p/base/transportchannelproxy.cc \
    $$PWD/p2p/base/transportdescription.cc \
    $$PWD/p2p/base/transport.cc \
    $$PWD/p2p/base/p2ptransport.cc \
    $$PWD/p2p/base/session.cc \
    $$PWD/p2p/base/sessiondescription.cc \
    $$PWD/p2p/base/dtlstransportchannel.cc \
    $$PWD/p2p/base/transportdescriptionfactory.cc \
    $$PWD/p2p/client/basicportallocator.cc \
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
    $$PWD/base/socketaddress.h \
    $$PWD/base/ipaddress.h \
    $$PWD/base/nethelpers.h \
    $$PWD/base/signalthread.h \
    $$PWD/base/sigslot.h \
    $$PWD/base/messagehandler.h \
    $$PWD/base/messagequeue.h \
    $$PWD/base/thread.h \
    $$PWD/base/timeutils.h \
    $$PWD/base/physicalsocketserver.h \
    $$PWD/base/common.h \
    $$PWD/base/asyncsocket.h \
    $$PWD/base/logging.h \
    $$PWD/base/stream.h \
    $$PWD/base/stringencode.h \
    $$PWD/p2p/base/basicpacketsocketfactory.h \
    $$PWD/base/network.h \
    $$PWD/base/virtualsocketserver.h \
    $$PWD/p2p/base/stun.h \
    $$PWD/p2p/base/stunport.h \
    $$PWD/p2p/base/stunrequest.h \
    $$PWD/p2p/base/stunserver.h \
    $$PWD/base/asyncudpsocket.h \
    $$PWD/base/helpers.h \
    $$PWD/p2p/base/port.h \
    $$PWD/base/asynctcpsocket.h \
    $$PWD/p2p/base/asyncstuntcpsocket.h \
    $$PWD/base/ssladapter.h \
    $$PWD/base/socketadapters.h \
    $$PWD/base/socketaddresspair.h \
    $$PWD/base/messagedigest.h \
    $$PWD/base/bytebuffer.h \
    $$PWD/base/crc32.h \
    $$PWD/base/base64.h \
    $$PWD/base/ratetracker.h \
    $$PWD/base/md5.h \
    $$PWD/base/sha1.h \
    $$PWD/base/httpcommon.h \
    $$PWD/p2p/base/portallocator.h \
    $$PWD/p2p/client/basicportallocator.h \
    $$PWD/base/firewallsocketserver.h \
    $$PWD/base/natsocketfactory.h \
    $$PWD/base/natserver.h \
    $$PWD/base/proxyserver.h \
    $$PWD/p2p/base/relayserver.h \
    $$PWD/p2p/base/p2ptransportchannel.h \
    $$PWD/p2p/base/portallocatorsessionproxy.h \
    $$PWD/p2p/base/tcpport.h \
    $$PWD/p2p/base/relayport.h \
    $$PWD/p2p/base/turnport.h \
    $$PWD/p2p/base/transportchannel.h \
    $$PWD/p2p/base/transportchannelimpl.h \
    $$PWD/base/nattypes.h \
    $$PWD/p2p/base/portproxy.h \
    $$PWD/p2p/base/pseudotcp.h \
    ../talk/p2p/base/transportchannelproxy.h \
    ../talk/p2p/base/transportdescription.h \
    ../talk/p2p/base/transport.h \
    ../talk/p2p/base/p2ptransport.h \
    ../talk/p2p/base/session.h \
    ../talk/p2p/base/constants.h \
    ../talk/p2p/base/sessiondescription.h \
    ../talk/p2p/base/dtlstransport.h \
    ../talk/p2p/base/dtlstransportchannel.h \
    ../talk/base/sslstreamadapter.h \
    ../talk/base/json.h \
    ../talk/app/webrtc/jsep.h \
    ../talk/base/stringutils.h \
    ../talk/p2p/base/transportdescriptionfactory.h \
    ../talk/app/webrtc/jsepicecandidate.h \
    ../talk/app/webrtc/jsepsessiondescription.h \
    ../talk/media/base/codec.h \
    ../talk/media/sctp/sctpdataengine.h \
    ../talk/media/sctp/sctputils.h \
    ../talk/app/webrtc/webrtcsdp.h \
    ../talk/app/kaerp2p/KaerSession.h \
    ../talk/app/kaerp2p/kaer_session_client.h \
    ../talk/session/tunnel/pseudotcpchannel.h \
    ../talk/session/tunnel/tunnelsessionclient.h \
    ../talk/app/kaerp2p/kaersessiondescriptionfactory.h




