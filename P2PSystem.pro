TEMPLATE = subdirs

SUBDIRS += \
    libjingle_app \
    third_party \
    talk






win32 {
    SUBDIRS += PCPClient
}
else{
   SUBDIRS += PCPLinux

}
