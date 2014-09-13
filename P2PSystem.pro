TEMPLATE = subdirs

SUBDIRS += \
    third_party \
    talk \
    webrtc/system_wrappers \
    libjingle_app \
    hisi



hisi {
  SUBDIRS +=  zmqclient \
    hisi

}


android {
     SUBDIRS += zmqclient
}else {
    SUBDIRS += TerminalSimulator
}

win32 {
}
else{

}
