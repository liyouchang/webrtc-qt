TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


DESTDIR = $$PWD/../bin

win32 {

Debug:DEFINES +=_DEBUG
Release:DEFINES +=
DEFINES += _UNICODE UNICODE WIN32_LEAN_AND_MEAN


} else:mac {
} else {
    Debug:DEFINES +=_DEBUG
    Release:DEFINES +=
    DEFINES += POSIX
}


INCLUDEPATH     += $$PWD/.. \
    ../third_party/jsoncpp/overrides/include ../third_party/jsoncpp/source/include \

DEFINES += JSONCPP_RELATIVE_PATH

win32 {

LIBS +=-lwinmm -liphlpapi -lcomsupp -lsecur32 -lws2_32  -lcrypt32 -lAdvapi32 -luser32

} else:mac {
} else {
    QMAKE_CXXFLAGS += -std=c++11
    LIBS += -pthread
}


LIBS += -L$$DESTDIR  \
        -ljsoncpp -llibjingle -llibjingle_p2p -llibjingle_app



SOURCES += \
    main.cpp \
    peer_connection_client.cc \
    defaults.cc

HEADERS += \
    peer_connection_client.h \
    defaults.h

