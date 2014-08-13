TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


#include (../socket/sockethandle.pri)
LIBS += -lws2_32

SOURCES += main.cpp

