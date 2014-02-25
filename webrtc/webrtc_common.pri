
DESTDIR = $$PWD/../libs

OBJECTS_DIR = $$PWD/../tmp/$$TARGET

webrtc_root = $$PWD/..

INCLUDEPATH += $$webrtc_root $$PWD


win32 {
DEFINES += WEBRTC_WIN
}

linux {

DEFINES += WEBRTC_LINUX

}
