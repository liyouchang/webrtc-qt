#-------------------------------------------------
#
# Project created by QtCreator 2014-02-15T09:35:39
#
#-------------------------------------------------

QT       -= core gui

TARGET = jingle
TEMPLATE = lib
CONFIG += staticlib

CONFIG -= app_bundle
CONFIG -= qt


include(talk_common.pri)

DESTDIR = $$output_dir/libs


DEFINES += SSL_USE_SCHANNEL

INCLUDEPATH     += ../third_party/jsoncpp/overrides/include \
                   ../third_party/jsoncpp/source/include



#operating system depend files
win32 {

SOURCES += \
    ../talk/base/win32securityerrors.cc \
    ../talk/base/win32socketinit.cc \
    ../talk/base/winping.cc \
    ../talk/base/win32socketserver.cc \
    ../talk/base/win32window.cc \
    ../talk/base/schanneladapter.cc \
    ../talk/base/win32.cc \
    base/win32windowpicker.cc \
    base/win32filesystem.cc


HEADERS += \
    ../talk/base/win32socketserver.h \
    ../talk/base/win32window.h \
    ../talk/base/winping.h \
    ../talk/base/win32socketinit.h \
    ../talk/base/win32.h \
    ../talk/base/schanneladapter.h \
    base/win32filesystem.h \
    base/win32windowpicker.h


}else{

SOURCES += \
    ../talk/base/posix.cc \
    ../talk/base/linux.cc \
    ../talk/base/linuxfdwalk.c \
    base/unixfilesystem.cc \
    base/latebindingsymboltable.cc \


HEADERS += \
    ../talk/base/posix.h \
    ../talk/base/linux.h \
    ../talk/base/linuxfdwalk.h \
    base/unixfilesystem.h \
    base/latebindingsymboltable.h \



}

SOURCES += \
    ../talk/base/socketaddress.cc \
    ../talk/base/ipaddress.cc \
    ../talk/base/nethelpers.cc \
    ../talk/base/signalthread.cc \
    ../talk/base/messagehandler.cc \
    ../talk/base/messagequeue.cc \
    ../talk/base/thread.cc \
    ../talk/base/timeutils.cc \
    ../talk/base/physicalsocketserver.cc \
    ../talk/base/common.cc \
    ../talk/base/asyncsocket.cc \
    ../talk/base/logging.cc \
    ../talk/base/stream.cc \
    ../talk/base/stringencode.cc \
    ../talk/base/network.cc \
    ../talk/base/virtualsocketserver.cc \
    ../talk/base/asyncudpsocket.cc \
    ../talk/base/helpers.cc \
    ../talk/base/socketadapters.cc \
    ../talk/base/socketaddresspair.cc \
    ../talk/base/messagedigest.cc \
    ../talk/base/bytebuffer.cc \
    ../talk/base/crc32.cc \
    ../talk/base/base64.cc \
    ../talk/base/ratetracker.cc \
    ../talk/base/md5.cc \
    ../talk/base/sha1.cc \
    ../talk/base/httpcommon.cc \
    ../talk/base/firewallsocketserver.cc \
    ../talk/base/natsocketfactory.cc \
    ../talk/base/natserver.cc \
    ../talk/base/proxyserver.cc \
    ../talk/base/nattypes.cc \
    ../talk/base/sslstreamadapter.cc \
    ../talk/base/json.cc \
    ../talk/base/stringutils.cc \
    ../talk/base/asynctcpsocket.cc \
    ../talk/base/asyncfile.cc \
    ../talk/base/sslfingerprint.cc \
    base/flags.cc \
    base/checks.cc \
    base/timing.cc \
    base/sslidentity.cc \
    base/cpumonitor.cc \
    base/fileutils.cc \
    base/pathutils.cc \
    base/systeminfo.cc \
    base/urlencode.cc \
    base/ratelimiter.cc \
    base/worker.cc \



HEADERS += \
    ../talk/base/socketaddress.h \
    ../talk/base/ipaddress.h \
    ../talk/base/nethelpers.h \
    ../talk/base/signalthread.h \
    ../talk/base/sigslot.h \
    ../talk/base/messagehandler.h \
    ../talk/base/messagequeue.h \
    ../talk/base/thread.h \
    ../talk/base/timeutils.h \
    ../talk/base/physicalsocketserver.h \
    ../talk/base/common.h \
    ../talk/base/asyncsocket.h \
    ../talk/base/logging.h \
    ../talk/base/stream.h \
    ../talk/base/stringencode.h \
    ../talk/base/network.h \
    ../talk/base/virtualsocketserver.h \
    ../talk/base/asyncudpsocket.h \
    ../talk/base/helpers.h \
    ../talk/base/asynctcpsocket.h \
    ../talk/base/socketadapters.h \
    ../talk/base/socketaddresspair.h \
    ../talk/base/messagedigest.h \
    ../talk/base/bytebuffer.h \
    ../talk/base/crc32.h \
    ../talk/base/base64.h \
    ../talk/base/ratetracker.h \
    ../talk/base/md5.h \
    ../talk/base/sha1.h \
    ../talk/base/httpcommon.h \
    ../talk/base/firewallsocketserver.h \
    ../talk/base/natsocketfactory.h \
    ../talk/base/natserver.h \
    ../talk/base/proxyserver.h \
    ../talk/base/nattypes.h \
    ../talk/base/sslstreamadapter.h \
    ../talk/base/json.h \
    ../talk/base/stringutils.h \
    ../talk/base/asyncfile.h \
    ../talk/base/sslfingerprint.h \
    ../talk/base/sigslotrepeater.h \
    base/flags.h \
    base/asyncinvoker-inl.h \
    base/checks.h \
    base/timing.h \
    base/sslidentity.h \
    base/cpumonitor.h \
    base/fileutils.h \
    base/pathutils.h \
    base/systeminfo.h \
    base/urlencode.h \
    base/ratelimiter.h \
    base/worker.h \
    base/buffer.h

