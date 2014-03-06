#-------------------------------------------------
#
# Project created by QtCreator 2014-03-03T10:36:53
#
#-------------------------------------------------

QT       -= core gui

TARGET = jingle_media_base
TEMPLATE = lib
CONFIG += staticlib


include (../talk/talk_common.pri)
DESTDIR = $$output_dir/libs

DEFINES -= LINUX


INCLUDEPATH  += ../third_party/libyuv/include  ../third_party/usrsctp \
    third_party/libudev

SOURCES += \
    ../talk/media/base/constants.cc \
    ../talk/media/base/codec.cc \
    ../talk/media/base/videoframe.cc \
    ../talk/media/base/yuvframegenerator.cc \
    ../talk/media/base/videocommon.cc \
    ../talk/media/base/videocapturer.cc \
    ../talk/media/base/streamparams.cc \
    ../talk/media/base/videoadapter.cc \
    ../talk/media/base/rtputils.cc \
    ../talk/media/base/rtpdump.cc \
    ../talk/media/base/rtpdataengine.cc \
    ../talk/media/base/mutedvideocapturer.cc \
    ../talk/media/base/filemediaengine.cc \
    ../talk/media/base/hybridvideoengine.cc \
    ../talk/media/base/mediaengine.cc \
    ../talk/media/base/capturemanager.cc \
    ../talk/media/base/capturerenderadapter.cc \
    ../talk/media/base/cpuid.cc \
    ../talk/media/devices/devicemanager.cc \
    ../talk/media/devices/yuvframescapturer.cc \
    ../talk/media/devices/filevideocapturer.cc \



HEADERS += \
    ../talk/media/base/constants.h \
    ../talk/media/base/codec.h \
    ../talk/media/base/videoframe.h \
    ../talk/media/base/videoprocessor.h \
    ../talk/media/base/videorenderer.h \
    ../talk/media/base/voiceprocessor.h \
    ../talk/media/base/yuvframegenerator.h \
    ../talk/media/base/videocommon.h \
    ../talk/media/base/videocapturer.h \
    ../talk/media/base/streamparams.h \
    ../talk/media/base/videoadapter.h \
    ../talk/media/base/rtputils.h \
    ../talk/media/base/screencastid.h \
    ../talk/media/base/rtpdump.h \
    ../talk/media/base/rtpdataengine.h \
    ../talk/media/base/mutedvideocapturer.h \
    ../talk/media/base/nullvideoframe.h \
    ../talk/media/base/nullvideorenderer.h \
    ../talk/media/base/filemediaengine.h \
    ../talk/media/base/hybriddataengine.h \
    ../talk/media/base/hybridvideoengine.h \
    ../talk/media/base/mediachannel.h \
    ../talk/media/base/mediacommon.h \
    ../talk/media/base/mediaengine.h \
    ../talk/media/base/audioframe.h \
    ../talk/media/base/audiorenderer.h \
    ../talk/media/base/capturemanager.h \
    ../talk/media/base/capturerenderadapter.h \
    ../talk/media/base/cpuid.h \
    ../talk/media/base/cryptoparams.h \
    ../talk/media/base/fakecapturemanager.h \
    ../talk/media/base/fakemediaengine.h \
    ../talk/media/base/fakemediaprocessor.h \
    ../talk/media/base/fakenetworkinterface.h \
    ../talk/media/base/fakertp.h \
    ../talk/media/base/fakevideocapturer.h \
    ../talk/media/base/fakevideorenderer.h \
    ../talk/media/devices/devicemanager.h \
    ../talk/media/devices/yuvframescapturer.h \
    ../talk/media/devices/filevideocapturer.h \



win32 {

SOURCES += \
            '../talk/media/devices/win32deviceinfo.cc'\
            '../talk/media/devices/win32devicemanager.cc'\



HEADERS += \
            '../talk/media/devices/win32devicemanager.h'\


}

linux {
SOURCES += \
    ../talk/media/devices/libudevsymboltable.cc \
    ../talk/media/devices/linuxdeviceinfo.cc

HEADERS += \
    ../talk/media/devices/libudevsymboltable.h

}
