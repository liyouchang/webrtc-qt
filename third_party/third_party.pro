TEMPLATE = subdirs

SUBDIRS += \
    jsoncpp \
    libyuv \
    libsrtp \
    cppzmq


win32 {
SUBDIRS += \
    winsdk_samples
}
