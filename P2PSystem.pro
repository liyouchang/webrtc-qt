TEMPLATE = subdirs

SUBDIRS += \
    jsoncpp \
    libjingle \
    libjingle_p2p \
    libjingle_app \
    PCPLinux \


win32 {
    SUBDIRS += PCPClient \
            PCPServer
}
