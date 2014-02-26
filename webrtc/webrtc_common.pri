
DESTDIR = $$PWD/../libs

OBJECTS_DIR = $$PWD/../tmp/$$TARGET

webrtc_root = $$PWD/..

INCLUDEPATH += $$webrtc_root $$PWD


win32 {
DEFINES += WEBRTC_WIN NOMINMAX  _UNICODE


}

linux {

DEFINES += WEBRTC_LINUX

}
