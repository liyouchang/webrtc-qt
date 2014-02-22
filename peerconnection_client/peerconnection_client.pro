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
    LIBS += -L$$PWD/../libs \
        -ljingle_peerconnection -ljingle_p2p \
        -ljingle_media -ljingle_sound  -ljingle \
        -ljsoncpp -lyuv -lsrtp
}

linux {

    PKGCONFIG =  glib-2.0 gobject-2.0 gtk+-2.0 gthread-2.0

    LIBS += -lX11 -lXcomposite -lXext -lXrender  -lrt

LIBS += -L$$PWD/../libs \
    -ljingle_peerconnection -ljingle_p2p \
    -ljingle_media -ljingle_sound  -ljingle \
    -ljsoncpp -lyuv -lsrtp -lcommon_video -lvideo_capture_module


    LIBS += -ldl

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
