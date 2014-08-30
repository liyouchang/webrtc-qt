

INCLUDEPATH += $$PWD/.. \
    ../third_party/jsoncpp/overrides/include ../third_party/jsoncpp/source/include


DEFINES += JSONCPP_RELATIVE_PATH LOGGING=1  \
    LIBPEERCONNECTION_LIB=1  USE_WEBRTC_DEV_BRANCH \
    SRTP_RELATIVE_PATH HAVE_SRTP





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
    DEFINES += POSIX OSX
    output_dir = $$PWD/../out/mac
#    DESTDIR = $$output_dir/libs

}
ios {
    DEFINES += POSIX IOS
    output_dir = $$PWD/../out/ios
    DESTDIR = $$output_dir/libs
}

linux {
    DEFINES += POSIX LINUX
    output_dir = $$PWD/../out
#    DESTDIR = $$output_dir/libs
}

hisi {
    DEFINES += ARM
    output_dir = $$PWD/../out/arm
}


android{
    output_dir = $$PWD/../out/android
}
#OBJECTS_DIR = $$output_dir/obj/$$TARGET

#unix {
#    target.path = /usr/lib
#    INSTALLS += target
#}
