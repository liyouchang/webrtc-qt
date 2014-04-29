TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


include (../../talk_common.pri)



DESTDIR = $$output_dir


win32 {

LIBS +=-lwinmm -liphlpapi -lcomsupp -lsecur32 -lws2_32  -lcrypt32 -lAdvapi32 -luser32
LIBS += -lStrmiids -lshell32 -lrpcrt4

    LIBS += -L$$output_dir/libs
    LIBS += -ljsoncpp
    LIBS += -ljingle_app  -ljingle_p2p  -ljingle



} else {
    #QMAKE_CXXFLAGS += -std=c++11

    LIBS += -L$$output_dir/libs
    LIBS +=   -ljingle_app  -ljingle_p2p  -ljingle
    LIBS += -pthread -ldl
}

SOURCES += \
    stunserver_main.cc



