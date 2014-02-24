#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T09:47:15
#
#-------------------------------------------------

QT       -= core gui

TARGET = common_audio_neon
TEMPLATE = lib
CONFIG += staticlib


include (../webrtc_common.pri)

SOURCES += \
            'resampler/sinc_resampler_neon.cc'\
            'signal_processing/cross_correlation_neon.S'\
            'signal_processing/downsample_fast_neon.S'\
            'signal_processing/min_max_operations_neon.S'\
            'signal_processing/vector_scaling_operations_neon.S'\


HEADERS +=
