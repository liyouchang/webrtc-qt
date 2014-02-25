#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T15:19:14
#
#-------------------------------------------------

QT       -= core gui

TARGET = video_processing
TEMPLATE = lib
CONFIG += staticlib

include (../../webrtc_common.pri)



SOURCES += \
    main/source/brighten.cc \
    main/source/brightness_detection.cc \
    main/source/color_enhancement.cc \
    main/source/content_analysis.cc \
    main/source/deflickering.cc \
    main/source/denoising.cc \
    main/source/frame_preprocessor.cc \
    main/source/spatial_resampler.cc \
    main/source/video_decimator.cc \
    main/source/video_processing_impl.cc

HEADERS += \
    main/source/brighten.h \
    main/source/brightness_detection.h \
    main/source/color_enhancement_private.h \
    main/source/color_enhancement.h \
    main/source/content_analysis.h \
    main/source/deflickering.h \
    main/source/denoising.h \
    main/source/frame_preprocessor.h \
    main/source/spatial_resampler.h \
    main/source/video_decimator.h \
    main/source/video_processing_impl.h



linux {

QMAKE_CFLAGS += -msse2

SOURCES += \
    main/source/content_analysis_sse2.cc \

}
OTHER_FILES += \
    main/source/video_processing.gypi \
    main/source/Android.mk
