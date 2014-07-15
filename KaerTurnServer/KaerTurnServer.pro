TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


include (../talk/talk_common.pri)

DESTDIR = $$output_dir

TARGET = kaerturnserver

INCLUDEPATH  +=  \
    ../third_party/jsoncpp/overrides/include ../third_party/jsoncpp/source/include \
    ../third_party/cppzmq/include


win32 {
    DEFINES += __WINDOWS__ ZMQ_STATIC

    LIBS +=-lwinmm -liphlpapi -lcomsupp -lsecur32 -lws2_32  -lcrypt32 -lAdvapi32 -luser32
    LIBS += -lStrmiids -lshell32 -lrpcrt4
    LIBS += -L$$output_dir/libs
    LIBS += -ljsoncpp -lcppzmq
    LIBS += -ljingle_p2p  -ljingle
} else {
    #QMAKE_CXXFLAGS += -std=c++11
    LIBS += -L$$output_dir/libs
    LIBS += -ljsoncpp -lcppzmq
    LIBS += -ljingle_p2p  -ljingle
    LIBS += -pthread -ldl
}

SOURCES += \
    turnserver.cc \
    turnserver_main.cc \
    asyndealer.cpp

HEADERS += \
    turnserver.h \
    asyndealer.h

OTHER_FILES += \
    turnserver_stop.sh \
    turnserver_start.sh

