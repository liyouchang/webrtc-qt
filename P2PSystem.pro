TEMPLATE = subdirs

SUBDIRS += \
    third_party \
    talk \
    libjingle_app



hisi {
  SUBDIRS +=  zmqclient
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
