TEMPLATE = subdirs

SUBDIRS += \
    jsoncpp \
    libjingle \
    libjingle_p2p \
    libjingle_app \





win32 {
    SUBDIRS += PCPClient
}
else{
   SUBDIRS += PCPLinux

}
