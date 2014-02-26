

debug {
    DEFINES += _DEBUG
    output_dir = $$PWD/out/debug
    OBJECTS_DIR = $$PWD/obj/debug/$$TARGET
}

release {
    output_dir = $$PWD/out/release
    OBJECTS_DIR = $$PWD/obj/release/$$TARGET

}

win32 {

DEFINES += _UNICODE UNICODE WIN32_LEAN_AND_MEAN

} else:macx {
}

linux {
    DEFINES += POSIX LINUX
}
