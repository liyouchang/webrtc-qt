
DESTDIR = $$PWD/../libs

OBJECTS_DIR = $$PWD/../tmp/$$TARGET

INCLUDEPATH += $$PWD/..


win32 {
DEFINES += WEBRTC_WIN
}

linux {

DEFINES += WEBRTC_LINUX

}
