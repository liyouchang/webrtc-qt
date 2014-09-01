TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt



include (../talk/talk_common.pri)
include (zmqdealer.pri)

DESTDIR = $$output_dir/$$TARGET

#message("DESTDIR is " $$DESTDIR)

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
#message (" this is linux system")
}

macx {
    LIBS += -L$$output_dir/libs
    LIBS += -ljingle_app -ljingle_p2p -ljingle
    LIBS += -lcppzmq -ljsoncpp
    LIBS +=  -ldl
    LIBS += -framework Foundation -framework Carbon

}

android {

SOURCES += \
    jni/JniPeerConnection.cpp \
    jni/JniUtil.cpp \
    jni/KeJniTunnelClient.cpp \
    jni/VideoPhone.cpp

HEADERS += \
    jni/JniPeerConnection.h \
    jni/JniUtil.h \
    jni/KeJniTunnelClient.h

OTHER_FILES += \
    jni/Android.mk

}

hisi {
    SOURCES += kesdkdevice.cpp
    HEADERS += kesdkdevice.h \
            keapi/media_define.h \
            keapi/media_api.h \
            keapi/common_define.h \
            keapi/common_api.h \
            keapi/store_api.h \
            keapi/store_define.h \
            keapi/web_api.h \
            keapi/web_define.h \
            keapi/alarm_api.h \
            keapi/alarm_define.h

    LIBS +=-L$$output_dir/libs/sdk -lweb  -lstore  -lmedia  -lalarm -lcommon
    LIBS += $$output_dir/libs/mpp/openssl-0.9.8d/libcrypto.a
    LIBS +=-L$$output_dir/libs/mpp/lib  -lmpi -lisp -lsns_ov9712_8 \
         -laec -lVoiceEngine -lanr -lresampler -laacenc


    target.path = /var/lib/tftpboot
    INSTALLS += target
    target_config.files  = $$PWD/config.json
    target_config.path   = /var/lib/tftpboot  $$output_dir
    INSTALLS  += target_config

#    SOURCES += HisiMediaDevice.cpp
#    HEADERS += HisiMediaDevice.h \
#            keapi/RayCommIPC_ParamInfo.h \
#            keapi/keapi.h
#    LIBS += -lkeapi -lstore -lexfat
}

OTHER_FILES += \
    config.json


SOURCES += \
    zmqclient.cpp \


