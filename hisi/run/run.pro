TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

include (../hisi_common.pri)

DESTDIR = $$output_dir

INCLUDEPATH += $$p2p_root/third_party/cppzmq/include\
    $$p2p_root/third_party/jsoncpp/overrides/include $$p2p_root/third_party/jsoncpp/source/include


DEFINES += JSONCPP_RELATIVE_PATH LOGGING=1

linux {
    #QMAKE_CXXFLAGS += -std=c++11
    LIBS += -L$$output_dir/libs
    LIBS += -ljingle -lsystem_wrappers

    LIBS += -lcppzmq -ljsoncpp
    LIBS += -pthread -ldl
}


hisi {
    HEADERS += \
            keapi/media_define.h \
            keapi/media_api.h \
            keapi/common_define.h \
            keapi/common_api.h \
            keapi/store_api.h \
            keapi/store_define.h \
            keapi/web_api.h \
            keapi/web_define.h \
            keapi/alarm_api.h \
            keapi/alarm_define.h \
            keapi/platform_api.h \
            keapi/platform_define.h


    LIBS +=-L$$output_dir/libs/sdk -lplatform -lweb  -lstore  -lmedia  -lalarm -lcommon
    LIBS += $$output_dir/libs/mpp/openssl-0.9.8d/libcrypto.a
    LIBS +=-L$$output_dir/libs/mpp/lib  -lmpi -lisp -lsns_ov9712_8 \
         -laec -lVoiceEngine -lanr -lresampler -laacenc


    target.path = /var/lib/tftpboot
    INSTALLS += target
    target_config.files  = $$PWD/config.json $$PWD/start_zmqclient.sh  $$PWD/stop_zmqclient.sh
    target_config.path   = /var/lib/tftpboot  $$output_dir
    INSTALLS  += target_config

#    SOURCES += HisiMediaDevice.cpp
#    HEADERS += HisiMediaDevice.h \
#            keapi/RayCommIPC_ParamInfo.h \
#            keapi/keapi.h
#    LIBS += -lkeapi -lstore -lexfat

OTHER_FILES += \
    start_zmqclient.sh \
    stop_zmqclient.sh

}



SOURCES += \
    run.cpp

