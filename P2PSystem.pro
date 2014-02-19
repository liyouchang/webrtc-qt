TEMPLATE = subdirs

SUBDIRS += \
    jsoncpp \
    libjingle \
    libjingle_p2p \
    libjingle_app \
    third_party \
    libjingle_sound





win32 {
    SUBDIRS += PCPClient
}
else{
   SUBDIRS += PCPLinux

}
