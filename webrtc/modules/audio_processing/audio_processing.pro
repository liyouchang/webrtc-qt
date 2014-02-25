#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T10:18:40
#
#-------------------------------------------------

QT       -= core gui

TARGET = audio_processing
TEMPLATE = lib
CONFIG += staticlib


include (../../webrtc_common.pri)

SOURCES += \
        'aec/echo_cancellation.c'\
        'aec/aec_core.c'\
        'aec/aec_rdft.c'\
        'aec/aec_resampler.c'\
        'aecm/echo_control_mobile.c'\
        'aecm/aecm_core.c'\
        'agc/analog_agc.c'\
        'agc/digital_agc.c'\
        'audio_buffer.cc'\
        'audio_processing_impl.cc'\
        'echo_cancellation_impl.cc'\
        'echo_control_mobile_impl.cc'\
        'gain_control_impl.cc'\
        'high_pass_filter_impl.cc'\
        'level_estimator_impl.cc'\
        'noise_suppression_impl.cc'\
        'processing_component.cc'\
        'typing_detection.cc'\
        'utility/delay_estimator.c'\
        'utility/delay_estimator_wrapper.c'\
        'utility/fft4g.c'\
        'utility/ring_buffer.c'\
        'voice_detection_impl.cc'\

HEADERS +=


QMAKE_CFLAGS += -msse2

SOURCES += \
            'aec/aec_core_sse2.c'\
            'aec/aec_rdft_sse2.c'\
    aecm/aecm_core_c.c


DEFINES += WEBRTC_NS_FLOAT
SOURCES += \
            'ns/noise_suppression.c'\
            'ns/ns_core.c'




OTHER_FILES += \
    audio_processing.gypi

