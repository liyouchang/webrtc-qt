#-------------------------------------------------
#
# Project created by QtCreator 2014-02-18T07:59:26
#
#-------------------------------------------------

QT       -= core gui

TARGET = jsoncpp
TEMPLATE = lib
CONFIG += staticlib


DESTDIR = $$PWD/../bin

INCLUDEPATH     += .. \
    ../third_party/jsoncpp/overrides/include \
    ../third_party/jsoncpp/source/include \
    ../third_party/jsoncpp/source/src/lib_json


HEADERS += \
    ../third_party/jsoncpp/source/include/json/assertions.h \
    ../third_party/jsoncpp/source/include/json/autolink.h \
    ../third_party/jsoncpp/source/include/json/config.h \
    ../third_party/jsoncpp/source/include/json/features.h \
    ../third_party/jsoncpp/source/include/json/forwards.h \
    ../third_party/jsoncpp/source/include/json/json.h \
    ../third_party/jsoncpp/source/include/json/reader.h \
    ../third_party/jsoncpp/source/include/json/value.h \
    ../third_party/jsoncpp/source/include/json/writer.h \
    ../third_party/jsoncpp/overrides/include/json/value.h \
    ../third_party/jsoncpp/source/src/lib_json/json_batchallocator.h \
    ../third_party/jsoncpp/source/src/lib_json/json_tool.h

SOURCES += \
    ../third_party/jsoncpp/overrides/src/lib_json/json_reader.cpp \
    ../third_party/jsoncpp/overrides/src/lib_json/json_value.cpp \
    ../third_party/jsoncpp/source/src/lib_json/json_writer.cpp


