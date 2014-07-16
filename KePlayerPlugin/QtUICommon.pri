

include (../talk/talk_common.pri)

DESTDIR = $$output_dir/$$TARGET

INCLUDEPATH     +=  $$PWD\
    ../third_party/jsoncpp/overrides/include ../third_party/jsoncpp/source/include \
    ../third_party/cppzmq/include

win32 {
#for zmqhelper
    DEFINES += __WINDOWS__ ZMQ_STATIC
    LIBS +=-lwinmm -liphlpapi -lcomsupp -lsecur32 -lws2_32  -lcrypt32 -lAdvapi32 -luser32
    LIBS += -lStrmiids -lshell32 -lrpcrt4
    LIBS += -L$$output_dir/libs
    LIBS += -ljsoncpp -lcppzmq
    LIBS += -ljingle_app  -ljingle_p2p  -ljingle
} else {
    #QMAKE_CXXFLAGS += -std=c++11
    LIBS += -L$$output_dir/libs
    LIBS +=   -ljingle_app  -ljingle_p2p  -ljingle
    LIBS += -ljsoncpp  -lcppzmq
    LIBS += -pthread -ldl
    #PRE_TARGETDEPS += $$DESTDIR/libjsoncpp.a $$DESTDIR/libjingle_p2p.a $$DESTDIR/libjingle.a $$DESTDIR/libjingle_app.a
}


include( $$PWD/KePlayView/KePlayView.pri)


SOURCES +=\
    ../zmqclient/asyndealer.cpp \
    ../zmqclient/peerconnectionclientdealer.cpp \
    $$PWD/KeQtTunnelClient.cpp \
    $$PWD/KePlayerPlugin.cpp \
    $$PWD/ke_recorder.cpp \
    ../KePlayerPlugin/kevideocutter.cpp

HEADERS  += \
    ../zmqclient/asyndealer.h \
    ../zmqclient/peerconnectionclientdealer.h \
    $$PWD/KeQtTunnelClient.h \
    $$PWD/KePlayerPlugin.h \
    $$PWD/ke_recorder.h \
    ../KePlayerPlugin/kevideocutter.h \
    $$PWD/ObjectSafetyImpl.h

