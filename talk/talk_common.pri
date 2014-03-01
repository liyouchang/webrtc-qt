





INCLUDEPATH += ..

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

linux {
    DEFINES += POSIX LINUX
    output_dir = $$PWD/../out

}

OBJECTS_DIR = $$output_dir/obj/$$TARGET

#unix {
#    target.path = /usr/lib
#    INSTALLS += target
#}
