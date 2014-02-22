


OBJECTS_DIR = ../tmp/$$TARGET


INCLUDEPATH     += ..

DEFINES += JSONCPP_RELATIVE_PATH LOGGING=1  \
    LIBPEERCONNECTION_LIB=1  USE_WEBRTC_DEV_BRANCH \
    SRTP_RELATIVE_PATH HAVE_SRTP

#',
#'    HAVE_WEBRTC_VIDEO HAVE_WEBRTC_VOICE \
#
debug {
    DEFINES += _DEBUG
}

release {
    DEFINES += _DEBUG
}

win32 {

Debug:DEFINES +=_DEBUG
Release:DEFINES +=
DEFINES += _UNICODE UNICODE WIN32_LEAN_AND_MEAN

} else:macx {
}

linux {
    DEFINES += POSIX LINUX
}

#unix {
#    target.path = /usr/lib
#    INSTALLS += target
#}
