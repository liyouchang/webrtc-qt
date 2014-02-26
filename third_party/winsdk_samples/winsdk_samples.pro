#-------------------------------------------------
#
# Project created by QtCreator 2014-02-26T14:23:05
#
#-------------------------------------------------

QT       -= core gui

TARGET = directshow_baseclasses
TEMPLATE = lib
CONFIG += staticlib

DESTDIR = $$PWD/../../libs

OBJECTS_DIR = ../../tmp/$$TARGET

DEFINES += UNICODE _UNICODE

baseclasses_dir = $$PWD/src/Samples/multimedia/directshow/baseclasses

INCLUDEPATH += $$baseclasses_dir


SOURCES += \
    src/Samples/multimedia/directshow/baseclasses/amextra.cpp \
    src/Samples/multimedia/directshow/baseclasses/amfilter.cpp \
    src/Samples/multimedia/directshow/baseclasses/amvideo.cpp \
    src/Samples/multimedia/directshow/baseclasses/arithutil.cpp \
    src/Samples/multimedia/directshow/baseclasses/combase.cpp \
    src/Samples/multimedia/directshow/baseclasses/cprop.cpp \
    src/Samples/multimedia/directshow/baseclasses/ctlutil.cpp \
    src/Samples/multimedia/directshow/baseclasses/ddmm.cpp \
    src/Samples/multimedia/directshow/baseclasses/dllentry.cpp \
    src/Samples/multimedia/directshow/baseclasses/dllsetup.cpp \
    src/Samples/multimedia/directshow/baseclasses/mtype.cpp \
    src/Samples/multimedia/directshow/baseclasses/outputq.cpp \
    src/Samples/multimedia/directshow/baseclasses/perflog.cpp \
    src/Samples/multimedia/directshow/baseclasses/pstream.cpp \
    src/Samples/multimedia/directshow/baseclasses/pullpin.cpp \
    src/Samples/multimedia/directshow/baseclasses/refclock.cpp \
    src/Samples/multimedia/directshow/baseclasses/renbase.cpp \
    src/Samples/multimedia/directshow/baseclasses/schedule.cpp \
    src/Samples/multimedia/directshow/baseclasses/seekpt.cpp \
    src/Samples/multimedia/directshow/baseclasses/source.cpp \
    src/Samples/multimedia/directshow/baseclasses/strmctl.cpp \
    src/Samples/multimedia/directshow/baseclasses/sysclock.cpp \
    src/Samples/multimedia/directshow/baseclasses/transfrm.cpp \
    src/Samples/multimedia/directshow/baseclasses/transip.cpp \
    src/Samples/multimedia/directshow/baseclasses/videoctl.cpp \
    src/Samples/multimedia/directshow/baseclasses/vtrans.cpp \
    src/Samples/multimedia/directshow/baseclasses/winctrl.cpp \
    src/Samples/multimedia/directshow/baseclasses/winutil.cpp \
    src/Samples/multimedia/directshow/baseclasses/wxdebug.cpp \
    src/Samples/multimedia/directshow/baseclasses/wxlist.cpp \
    src/Samples/multimedia/directshow/baseclasses/wxutil.cpp

HEADERS += \
    src/Samples/multimedia/directshow/baseclasses/amextra.h \
    src/Samples/multimedia/directshow/baseclasses/amfilter.h \
    src/Samples/multimedia/directshow/baseclasses/cache.h \
    src/Samples/multimedia/directshow/baseclasses/checkbmi.h \
    src/Samples/multimedia/directshow/baseclasses/combase.h \
    src/Samples/multimedia/directshow/baseclasses/cprop.h \
    src/Samples/multimedia/directshow/baseclasses/ctlutil.h \
    src/Samples/multimedia/directshow/baseclasses/ddmm.h \
    src/Samples/multimedia/directshow/baseclasses/dllsetup.h \
    src/Samples/multimedia/directshow/baseclasses/dxmperf.h \
    src/Samples/multimedia/directshow/baseclasses/fourcc.h \
    src/Samples/multimedia/directshow/baseclasses/measure.h \
    src/Samples/multimedia/directshow/baseclasses/msgthrd.h \
    src/Samples/multimedia/directshow/baseclasses/mtype.h \
    src/Samples/multimedia/directshow/baseclasses/outputq.h \
    src/Samples/multimedia/directshow/baseclasses/perflog.h \
    src/Samples/multimedia/directshow/baseclasses/perfstruct.h \
    src/Samples/multimedia/directshow/baseclasses/pstream.h \
    src/Samples/multimedia/directshow/baseclasses/pullpin.h \
    src/Samples/multimedia/directshow/baseclasses/refclock.h \
    src/Samples/multimedia/directshow/baseclasses/reftime.h \
    src/Samples/multimedia/directshow/baseclasses/renbase.h \
    src/Samples/multimedia/directshow/baseclasses/schedule.h \
    src/Samples/multimedia/directshow/baseclasses/seekpt.h \
    src/Samples/multimedia/directshow/baseclasses/source.h \
    src/Samples/multimedia/directshow/baseclasses/streams.h \
    src/Samples/multimedia/directshow/baseclasses/strmctl.h \
    src/Samples/multimedia/directshow/baseclasses/sysclock.h \
    src/Samples/multimedia/directshow/baseclasses/transfrm.h \
    src/Samples/multimedia/directshow/baseclasses/transip.h \
    src/Samples/multimedia/directshow/baseclasses/videoctl.h \
    src/Samples/multimedia/directshow/baseclasses/vtrans.h \
    src/Samples/multimedia/directshow/baseclasses/winctrl.h \
    src/Samples/multimedia/directshow/baseclasses/winutil.h \
    src/Samples/multimedia/directshow/baseclasses/wxdebug.h \
    src/Samples/multimedia/directshow/baseclasses/wxlist.h \
    src/Samples/multimedia/directshow/baseclasses/wxutil.h


OTHER_FILES += \
    winsdk_samples.gyp \
