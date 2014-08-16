



win32 {

DEFINES += _UNICODE UNICODE WIN32_LEAN_AND_MEAN
    Debug {
        DEFINES += _DEBUG
        output_dir = $$PWD/../out/debug
    }

    Release {
        output_dir = $$PWD/../out/release
    }

}

macx {
        output_dir = $$PWD/../out/mac
}

ios {
DEFINES += POSIX IOS
        output_dir = $$PWD/../out/ios
}

linux {
    DEFINES += POSIX LINUX
    output_dir = $$PWD/../out
}



arm {
    DEFINES += ARM
    output_dir = $$PWD/../out/arm
}

android{
    output_dir = $$PWD/../out/android

}

DESTDIR = $$output_dir/libs
OBJECTS_DIR = $$output_dir/obj/$$TARGET
