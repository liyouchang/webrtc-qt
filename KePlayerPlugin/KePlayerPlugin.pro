#-------------------------------------------------
#
# Project created by QtCreator 2014-04-09T10:32:01
#
#-------------------------------------------------
TEMPLATE = lib

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = KePlayerPlugin
CONFIG +=  dll


win32{
    RC_FILE  = KePlayerPlugin.rc
}
#DEF_FILE = KePlayerPlugin.def

include(npapi/qtbrowserplugin.pri)
include(QtUICommon.pri)

SOURCES +=\
    PluginMain.cpp

OTHER_FILES += \
    KePlayerPlugin.rc \
    pluginTest/KePlayerPlugin.html \
    pluginTest/js/jquery-1.11.0.js \
    pluginTest/js/keplayer.js \
    pluginTest/css/main.css \
    pluginTest/js/json2.js

#message( $$DESTDIR/$$TARGET".exe")
#target_config.files = $$DESTDIR/$$TARGET
target.path = $$PWD/setup
INSTALLS += target

htmltarget.files = pluginTest/*
htmltarget.path = $$PWD/setup/pluginTest
INSTALLS += htmltarget
