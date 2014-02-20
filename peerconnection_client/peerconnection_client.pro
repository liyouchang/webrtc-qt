TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

DESTDIR = $$PWD/../bin


include (../talk/talk_common.pri)

INCLUDEPATH     += ../third_party/jsoncpp/overrides/include \
                   ../third_party/jsoncpp/source/include

CONFIG += link_pkgconfig
PKGCONFIG =  glib-2.0 gobject-2.0 gtk+-2.0 gthread-2.0

win32 {
}
else {
    LIBS += -ldl -lX11 -lXcomposite -lXext -lXrender  -lrt

    LIBS += -L$$PWD/../libs -ljingle_peerconnection -ljingle_p2p -ljingle_media -ljingle_sound  -ljingle -ljsoncpp -lyuv
}
HEADERS += \
    peer_connection_client.h \
    defaults.h \
    conductor.h \


SOURCES += \
    peer_connection_client.cc \
    defaults.cc \
    conductor.cc \



#    ../talk/media/devices/dummydevicemanager.cc

win32 {
}else{

HEADERS += \
    linux/main_wnd.h


SOURCES += \
    linux/main_wnd.cc \
    linux/main.cc \

}
