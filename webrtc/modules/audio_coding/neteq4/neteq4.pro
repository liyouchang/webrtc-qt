#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T14:35:31
#
#-------------------------------------------------

QT       -= core gui

TARGET = NetEq4
TEMPLATE = lib
CONFIG += staticlib


include (../../../webrtc_common.pri)

SOURCES += \
    accelerate.cc \
    audio_decoder_impl.cc \
    audio_decoder.cc \
    audio_multi_vector.cc \
    audio_vector.cc \
    background_noise.cc \
    buffer_level_filter.cc \
    comfort_noise.cc \
    decision_logic_fax.cc \
    decision_logic_normal.cc \
    decision_logic.cc \
    decoder_database.cc \
    delay_manager.cc \
    delay_peak_detector.cc \
    dsp_helper.cc \
    dtmf_buffer.cc \
    dtmf_tone_generator.cc \
    expand.cc \
    merge.cc \
    neteq_impl.cc \
    neteq.cc \
    normal.cc \
    packet_buffer.cc \
    payload_splitter.cc \
    post_decode_vad.cc \
    preemptive_expand.cc \
    random_vector.cc \
    rtcp.cc \
    statistics_calculator.cc \
    sync_buffer.cc \
    time_stretch.cc \
    timestamp_scaler.cc

HEADERS += \
    accelerate.h \
    audio_decoder_impl.h \
    audio_multi_vector.h \
    audio_vector.h \
    background_noise.h \
    buffer_level_filter.h \
    comfort_noise.h \
    decision_logic_fax.h \
    decision_logic_normal.h \
    decision_logic.h \
    decoder_database.h \
    defines.h \
    delay_manager.h \
    delay_peak_detector.h \
    dsp_helper.h \
    dtmf_buffer.h \
    dtmf_tone_generator.h \
    expand.h \
    merge.h \
    neteq_impl.h \
    normal.h \
    packet_buffer.h \
    packet.h \
    payload_splitter.h \
    post_decode_vad.h \
    preemptive_expand.h \
    random_vector.h \
    rtcp.h \
    statistics_calculator.h \
    sync_buffer.h \
    time_stretch.h \
    timestamp_scaler.h

OTHER_FILES += \
    neteq.gypi \
    audio_decoder_unittests.isolate \
    neteq_tests.gypi \
    OWNERS
