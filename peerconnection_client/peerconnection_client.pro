TEMPLATE = app
#CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DESTDIR = $$PWD/../bin


include (../talk/talk_common.pri)

INCLUDEPATH     += ../third_party/jsoncpp/overrides/include \
                   ../third_party/jsoncpp/source/include

CONFIG += link_pkgconfig

debug{


}

release{

}


win32 {

INCLUDEPATH += $(VSInstallDir)\VC\atlmfc\include


    LIBS += -lwinmm -liphlpapi -lcomsupp -lsecur32 -lws2_32  -lcrypt32 -lAdvapi32 -luser32

    LIBS += -lgdi32 -lStrmiids
    #for audio device dependence
    LIBS += -ldmoguids -lwmcodecdspuuid -lamstrmid -lmsdmo

    LIBS += -L$$PWD/../libs

   LIBS +=  -lvideo_engine_core -lwebrtc_video_coding -lvideo_capture_module  \
        -lvoice_engine -laudio_coding_module\
      -lacm2    -lvideo_capture_module\
     -laudio_device -laudio_processing -lwebrtc_utility -lmedia_file\
    -lrtp_rtcp -laudio_conference_mixer -laudio_coding_module -lG722 -lG711 -lCNG\
    -liLBC -liSAC -lNetEq4 -lNetEq -lPCM16B \
    -lvideo_processing  -lvideo_render_module -lcommon_video -lpaced_sender \
    -lremote_bitrate_estimator -lvideo_coding_utility\
    -lbitrate_controller  -lwebrtc_i420 -lcommon_audio -lsystem_wrappers\


    LIBS +=-ljingle_media -ljingle_peerconnection -ljingle_p2p  -ljingle_sound  -ljingle \


    LIBS +=  -ljsoncpp -lyuv -lsrtp  -ldirectshow_baseclasses\

}

linux {

    PKGCONFIG =  glib-2.0 gobject-2.0 gtk+-2.0 gthread-2.0


LIBS += -L$$PWD/../libs \
    -ljingle_peerconnection -ljingle_p2p -ljingle_media -ljingle_sound  -ljingle \
    -lvideo_engine_core -lwebrtc_video_coding -lvideo_capture_module  \
    -lvoice_engine -laudio_coding_module\
      -lacm2    -lvideo_capture_module\
     -laudio_device -laudio_processing -lwebrtc_utility -lmedia_file\
    -lrtp_rtcp -laudio_conference_mixer -laudio_coding_module -lG722 -lG711 -lCNG\
    -liLBC -liSAC -lNetEq4 -lNetEq -lPCM16B \
    -lvideo_processing  -lvideo_render -lcommon_video -lpaced_sender -lremote_bitrate_estimator -lvideo_coding_utility\
    -lbitrate_controller  -lwebrtc_i420 -lcommon_audio -lsystem_wrappers\


    LIBS +=  -ljsoncpp -lyuv -lsrtp  \



    LIBS += -ldl -lm
    LIBS += -lX11 -lXcomposite -lXext -lXrender  -lrt

}


HEADERS += \
    peer_connection_client.h \
    defaults.h \
    conductor.h \
    flagdefs.h \


SOURCES += \
    peer_connection_client.cc \
    defaults.cc \
    conductor.cc \



#    ../talk/media/devices/dummydevicemanager.cc

win32 {
HEADERS += \
    main_wnd.h

SOURCES += \
    main_wnd.cc \
    main.cc


}else{

HEADERS += \
    linux/main_wnd.h \
    ../talk/base/linuxwindowpicker.h \


SOURCES += \
    linux/main_wnd.cc \
    linux/main.cc \
    ../talk/base/linuxwindowpicker.cc \


}
