#-------------------------------------------------
#
# Project created by QtCreator 2014-02-19T15:42:05
#
#-------------------------------------------------

QT       -= core gui

TARGET = jingle_media

TEMPLATE = lib
CONFIG += staticlib

include(talk_common.pri)

DESTDIR = $$output_dir/libs


DEFINES += HAVE_WEBRTC_VIDEO HAVE_WEBRTC_VOICE

INCLUDEPATH  += ../third_party/libyuv/include  ../third_party/usrsctp \
    third_party/libudev



HEADERS += \
        'media/base/audioframe.h'\
        'media/base/audiorenderer.h'\
        'media/base/capturemanager.h'\
        'media/base/capturerenderadapter.h'\
        'media/base/codec.h'\
        'media/base/cpuid.h'\
        'media/base/constants.h'\
        'media/base/cryptoparams.h'\
        'media/base/filemediaengine.h'\
        'media/base/hybriddataengine.h'\
        'media/base/hybridvideoengine.h'\
        'media/base/mediachannel.h'\
        'media/base/mediacommon.h'\
        'media/base/mutedvideocapturer.h'\
        'media/base/mediaengine.h'\
        'media/base/rtpdataengine.h'\
        'media/base/rtpdump.h'\
        'media/base/rtputils.h'\
        'media/base/screencastid.h'\
        'media/base/streamparams.h'\
        'media/base/videoadapter.h'\
        'media/base/videocommon.h'\
        'media/base/videocapturer.h'\
        'media/base/videoframe.h'\
        'media/base/videoprocessor.h'\
        'media/base/videorenderer.h'\
        'media/base/voiceprocessor.h'\
        'media/base/yuvframegenerator.h'\
        'media/devices/deviceinfo.h'\
        'media/devices/devicemanager.h'\
        'media/devices/dummydevicemanager.h'\
        'media/devices/filevideocapturer.h'\
        'media/devices/videorendererfactory.h'\
        'media/devices/yuvframescapturer.h'\
        'media/other/linphonemediaengine.h'\
        'media/sctp/sctpdataengine.h'\
        'media/webrtc/webrtccommon.h'\
        'media/webrtc/webrtcexport.h'\
        'media/webrtc/webrtcmediaengine.h'\
        'media/webrtc/webrtcpassthroughrender.h'\
        'media/webrtc/webrtctexturevideoframe.h'\
        'media/webrtc/webrtcvideocapturer.h'\
        'media/webrtc/webrtcvideodecoderfactory.h'\
        'media/webrtc/webrtcvideoencoderfactory.h'\
        'media/webrtc/webrtcvideoengine.h'\
        'media/webrtc/webrtcvideoframe.h'\
        'media/webrtc/webrtcvie.h'\
        'media/webrtc/webrtcvoe.h'\
        'media/webrtc/webrtcvoiceengine.h'\


SOURCES += \
        'media/base/capturemanager.cc'\
        'media/base/capturerenderadapter.cc'\
        'media/base/codec.cc'\
        'media/base/constants.cc'\
        'media/base/cpuid.cc'\
        'media/base/filemediaengine.cc'\
        'media/base/hybridvideoengine.cc'\
        'media/base/mediaengine.cc'\
        'media/base/mutedvideocapturer.cc'\
        'media/base/rtpdataengine.cc'\
        'media/base/rtpdump.cc'\
        'media/base/rtputils.cc'\
        'media/base/streamparams.cc'\
        'media/base/videoadapter.cc'\
        'media/base/videocapturer.cc'\
        'media/base/videocommon.cc'\
        'media/base/videoframe.cc'\
        'media/base/yuvframegenerator.cc'\
        'media/devices/devicemanager.cc'\
        'media/devices/filevideocapturer.cc'\
        'media/devices/yuvframescapturer.cc'\
        'media/sctp/sctpdataengine.cc'\
        'media/webrtc/webrtcpassthroughrender.cc'\
        'media/webrtc/webrtctexturevideoframe.cc'\
        'media/webrtc/webrtcvideocapturer.cc'\
        'media/webrtc/webrtcvideoframe.cc'\
        'media/webrtc/webrtcvoiceengine.cc'\
        media/webrtc/webrtcvideoengine.cc


win32 {
SOURCES += \
            'media/devices/gdivideorenderer.cc'\
            'media/devices/win32deviceinfo.cc'\
            'media/devices/win32devicemanager.cc'\


HEADERS += \
            'media/devices/win32devicemanager.h'\
            'media/devices/gdivideorenderer.h'\


}

linux {


HEADERS += \
            'media/devices/v4llookup.h'\
    media/devices/linuxdevicemanager.h \
    media/devices/libudevsymboltable.h


SOURCES += \
            'media/devices/v4llookup.cc'\
    media/devices/linuxdevicemanager.cc \
    media/devices/linuxdeviceinfo.cc \
    media/devices/libudevsymboltable.cc


}

