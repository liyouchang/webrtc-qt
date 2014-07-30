TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


include (../talk/talk_common.pri)
include (zmqdealer.pri)

DESTDIR = $$output_dir/$$TARGET

#message("DESTDIR is "$$DESTDIR)

win32 {
    #for zmqhelper
    DEFINES += __WINDOWS__ ZMQ_STATIC
    LIBS += -lwinmm -liphlpapi -lcomsupp -lsecur32 -lws2_32  -lcrypt32 -lAdvapi32 -luser32
    LIBS += -lStrmiids -lshell32 -lrpcrt4
    LIBS += -L$$output_dir/libs
    LIBS += -ljsoncpp -lcppzmq
    LIBS += -ljingle_app -ljingle_p2p -ljingle
} else {
    #QMAKE_CXXFLAGS += -std=c++11
    LIBS += -L$$output_dir/libs
    LIBS += -ljingle_app -ljingle_p2p -ljingle
    LIBS += -lcppzmq -ljsoncpp
    LIBS += -pthread -ldl
    #LIBS += -lprofiler
}

SOURCES += \
    zmqclient.cpp


hisi {
    SOURCES += HisiMediaDevice.cpp
    HEADERS += HisiMediaDevice.h \
            keapi/RayCommIPC_ParamInfo.h \
            keapi/keapi.h
    LIBS += -lkeapi -lstore -lexfat
}else {
#    SOURCES +=  KeVideoSimulator.cpp
#    HEADERS +=  KeVideoSimulator.h
}

OTHER_FILES += \
    config.json


hisi {
    target_config.files  = $$OTHER_FILES $$DESTDIR/$$TARGET
    target_config.path   = /var/lib/tftpboot
    INSTALLS  += target_config
}eles{
#    target.files = $$DESTDIR/$$TARGET
#    target.path =   /home/lht/workspace/P2PSystem/out/zmqclient
#    INSTALLS += target

##    target_config.files  = $$OTHER_FILES
##    target_config.path   = $$output_dir/$$TARGET
##    INSTALLS  += target_config

}
