TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


include (../talk/talk_common.pri)
include (../zmqclient/zmqdealer.pri)

DESTDIR = $$output_dir/$$TARGET

win32 {
    #for zmqhelper
    DEFINES += __WINDOWS__ ZMQ_STATIC
    LIBS += -lwinmm -liphlpapi -lcomsupp -lsecur32 -lws2_32  -lcrypt32 -lAdvapi32 -luser32
    LIBS += -lStrmiids -lshell32 -lrpcrt4
    LIBS += -L$$output_dir/libs
    LIBS += -ljsoncpp -lcppzmq
    LIBS += -ljingle_app -ljingle_p2p -ljingle
}
linux {
    #QMAKE_CXXFLAGS += -std=c++11
    LIBS += -L$$output_dir/libs
    LIBS += -ljingle_app -ljingle_p2p -ljingle
    LIBS += -lcppzmq -ljsoncpp
    LIBS += -pthread -ldl
    #LIBS += -lprofiler
}

macx {
    LIBS += -L$$output_dir/libs
    LIBS += -ljingle_app -ljingle_p2p -ljingle
    LIBS += -lcppzmq -ljsoncpp
    LIBS +=  -ldl
    LIBS += -framework Foundation -framework Carbon

}
ios {
    LIBS += -framework Foundation -lstdc++

    LIBS += -L$$output_dir/libs

    LIBS += -ljingle_app -ljingle_p2p -ljingle
    LIBS += -lcppzmq -ljsoncpp
    LIBS +=  -ldl
#    QMAKE_CXXFLAGS = -std=c++11 -stdlib=libstdc++
#    QMAKE_LFLAGS = -std=c++11 -stdlib=libstdc++
#                '-framework Foundation',
#                '-framework IOKit',
#                '-framework Security',
#                '-framework SystemConfiguration',
#                '-framework UIKit',

}

SOURCES += \
    TerminalSimulator.cpp \
    KeVideoSimulator.cpp

HEADERS += \
    KeVideoSimulator.h

OTHER_FILES += \
    config.json


#target.files = $$DESTDIR/$$TARGET
#target.path = $$output_dir/$$TARGET
#INSTALLS += target

target_config.files  = $$OTHER_FILES
target_config.path   = $$output_dir/$$TARGET
INSTALLS  += target_config

