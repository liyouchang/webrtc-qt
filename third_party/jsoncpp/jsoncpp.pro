#-------------------------------------------------
#
# Project created by QtCreator 2014-02-18T07:59:26
#
#-------------------------------------------------

QT       -= core gui

TARGET = jsoncpp
TEMPLATE = lib
CONFIG += staticlib

include( ../third_party.pri)

DEFINES += JSON_USE_EXCEPTION=0

INCLUDEPATH     += \
    overrides/include \
    source/include \
    source/src/lib_json


HEADERS += \
    source/include/json/assertions.h \
    source/include/json/autolink.h \
    source/include/json/config.h \
    source/include/json/features.h \
    source/include/json/forwards.h \
    source/include/json/json.h \
    source/include/json/reader.h \
    source/include/json/value.h \
    source/include/json/writer.h \
    overrides/include/json/value.h \
    source/src/lib_json/json_batchallocator.h \
    source/src/lib_json/json_tool.h

SOURCES += \
    overrides/src/lib_json/json_reader.cpp \
    overrides/src/lib_json/json_value.cpp \
    source/src/lib_json/json_writer.cpp

OTHER_FILES += \
    jsoncpp.gyp \
    jsoncpp.gypi


