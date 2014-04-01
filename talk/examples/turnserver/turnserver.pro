TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt



include (../../talk_common.pri)



DESTDIR = $$output_dir


win32 {

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
    turnserver_main.cc

