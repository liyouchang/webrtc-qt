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
include(../TunnelClientUI/QtUICommon.pri)


SOURCES +=\
        KePlayerPlugin.cpp \
    PluginMain.cpp

HEADERS  += KePlayerPlugin.h

OTHER_FILES += \
    KePlayerPlugin.rc \
    pluginTest/KePlayerPlugin.html
