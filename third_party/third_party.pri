



win32 {

DEFINES += _UNICODE UNICODE WIN32_LEAN_AND_MEAN
    Debug {
        DEFINES += _DEBUG
        output_dir = $$PWD/../out/debug
    }

    Release {
        output_dir = $$PWD/../out/release
    }

} else:macx {
}

linux {
    DEFINES += POSIX LINUX
    output_dir = $$PWD/../out
}



arm {
    DEFINES += ARM
    output_dir = $$PWD/../out/arm
}

DESTDIR = $$output_dir/libs
OBJECTS_DIR = $$output_dir/obj/$$TARGET
