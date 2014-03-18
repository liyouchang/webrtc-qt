TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


include (../talk/talk_common.pri)

DESTDIR = $$output_dir

INCLUDEPATH     +=  \
    ../third_party/jsoncpp/overrides/include ../third_party/jsoncpp/source/include \
    ../third_party/cppzmq/include




win32 {


#for zmqhelper
DEFINES += __WINDOWS__ ZMQ_STATIC

LIBS +=-lwinmm -liphlpapi -lcomsupp -lsecur32 -lws2_32  -lcrypt32 -lAdvapi32 -luser32
LIBS += -lStrmiids -lshell32 -lrpcrt4

    LIBS += -L$$output_dir/libs
    LIBS += -ljsoncpp
    LIBS += -ljingle_app  -ljingle_p2p  -ljingle
LIBS +=  -lcppzmq



} else {
    #QMAKE_CXXFLAGS += -std=c++11

    LIBS += -L$$output_dir/libs

    LIBS +=   -ljingle_app  -ljingle_p2p  -ljingle
    LIBS += -ljsoncpp  -lcppzmq

    LIBS += -pthread -ldl

    #PRE_TARGETDEPS += $$DESTDIR/libjsoncpp.a $$DESTDIR/libjingle_p2p.a $$DESTDIR/libjingle.a $$DESTDIR/libjingle_app.a

}

SOURCES += main.cpp \
    asyndealer.cpp \
    defaults.cc \
    p2pconductor.cpp \
    peertunnel.cpp \
    streamprocess.cpp \
    peerconnectionclientdealer.cpp \
    peerterminal.cpp

HEADERS += \
    asyndealer.h \
    defaults.h \
    p2pconductor.h \
    peertunnel.h \
    streamprocess.h \
    PeerConnectionClinetInterface.h \
    peerconnectionclientdealer.h \
    peerterminal.h

