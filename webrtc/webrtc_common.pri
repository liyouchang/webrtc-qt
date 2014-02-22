
DESTDIR = $$PWD/../libs

OBJECTS_DIR = $$PWD/../tmp/$$TARGET

INCLUDEPATH += $$PWD/..


win32 {
}

linux {

DEFINES += WEBRTC_LINUX

}
