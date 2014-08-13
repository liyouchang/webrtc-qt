TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp


hisi {
    #target.files  = $$DESTDIR/$$TARGET
    target.path   = /var/lib/tftpboot
    INSTALLS  += target
}
