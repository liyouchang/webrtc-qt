

webrtc_root = $$PWD/..

INCLUDEPATH += $$webrtc_root $$PWD



win32 {
    DEFINES += WEBRTC_WIN NOMINMAX  _UNICODE
    Debug {
        DEFINES += _DEBUG
        output_dir = $$webrtc_root/out/debug
    }
    Release {
        output_dir = $$webrtc_root/out/release
    }
}

linux {
    DEFINES += WEBRTC_LINUX
    output_dir = $$webrtc_root/out
}

DESTDIR = $$output_dir/libs
OBJECTS_DIR = $$output_dir/obj/$$TARGET
