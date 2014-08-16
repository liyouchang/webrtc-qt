QT       -= core gui

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
#CONFIG -= qt


output_dir = $$PWD/../../out/ios


OBJECTS_DIR = $$output_dir/obj/$$TARGET

DESTDIR = $$output_dir/tests

INCLUDEPATH     +=  \
    ../../third_party/cppzmq/include


LIBS += -L$$output_dir/libs
LIBS += -lcppzmq

SOURCES += main.cpp

