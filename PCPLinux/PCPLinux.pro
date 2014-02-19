TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


DESTDIR = $$PWD/../bin

win32 {

Debug:DEFINES +=_DEBUG
Release:DEFINES +=
DEFINES += _UNICODE UNICODE WIN32_LEAN_AND_MEAN

} else {
    DEFINES += POSIX LOGGING=1
}


INCLUDEPATH     += $$PWD/.. \
    ../third_party/jsoncpp/overrides/include ../third_party/jsoncpp/source/include \

DEFINES += JSONCPP_RELATIVE_PATH

win32 {

LIBS +=-lwinmm -liphlpapi -lcomsupp -lsecur32 -lws2_32  -lcrypt32 -lAdvapi32 -luser32

} else {
    #QMAKE_CXXFLAGS += -std=c++11
    LIBS += -pthread

    LIBS += -L$$PWD/../lib  \
         -ljingle_app -ljingle_p2p -ljingle -ljsoncpp


    #PRE_TARGETDEPS += $$DESTDIR/libjsoncpp.a $$DESTDIR/libjingle_p2p.a $$DESTDIR/libjingle.a $$DESTDIR/libjingle_app.a

}





SOURCES += \
    main.cpp \
    peer_connection_client.cc \
    defaults.cc \
    ServerConductor.cpp

HEADERS += \
    peer_connection_client.h \
    defaults.h \
    ServerConductor.h

