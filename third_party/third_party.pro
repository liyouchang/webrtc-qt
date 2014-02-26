TEMPLATE = subdirs

SUBDIRS += \
    jsoncpp \
    libyuv \
    libsrtp \



win32 {
SUBDIRS += \
    winsdk_samples
}
