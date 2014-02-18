TEMPLATE = subdirs

SUBDIRS += \
    libjingle \
    libjingle_p2p \
    libjingle_app \
    PCPLinux \
    jsoncpp


win32 {
    SUBDIRS += PCPClient \
            PCPServer
}
