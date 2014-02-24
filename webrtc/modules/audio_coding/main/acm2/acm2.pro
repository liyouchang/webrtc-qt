#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T09:08:28
#
#-------------------------------------------------

QT       -= core gui

TARGET = acm2
TEMPLATE = lib
CONFIG += staticlib


include (../../../../webrtc_common.pri)

SOURCES += \
    acm_amr.cc \
    acm_amrwb.cc \
    acm_celt.cc \
    acm_cng.cc \
    acm_codec_database.cc \
    acm_dtmf_playout.cc \
    acm_g722.cc \
    acm_g729.cc \
    acm_g7221.cc \
    acm_g7221c.cc \
    acm_g7291.cc \
    acm_generic_codec.cc \
    acm_gsmfr.cc \
    acm_ilbc.cc \
    acm_isac.cc \
    acm_opus.cc \
    acm_pcm16b.cc \
    acm_pcma.cc \
    acm_pcmu.cc \
    acm_receiver.cc \
    acm_red.cc \
    acm_resampler.cc \
    acm_speex.cc \
    audio_coding_module_impl.cc \
    audio_coding_module.cc \
    call_statistics.cc \
    initial_delay_manager.cc \
    nack.cc

HEADERS += \
    acm_amr.h \
    acm_amrwb.h \
    acm_celt.h \
    acm_cng.h \
    acm_codec_database.h \
    acm_common_defs.h \
    acm_dtmf_playout.h \
    acm_g722.h \
    acm_g729.h \
    acm_g7221.h \
    acm_g7221c.h \
    acm_g7291.h \
    acm_generic_codec.h \
    acm_gsmfr.h \
    acm_ilbc.h \
    acm_isac_macros.h \
    acm_isac.h \
    acm_opus.h \
    acm_pcm16b.h \
    acm_pcma.h \
    acm_pcmu.h \
    acm_receiver.h \
    acm_red.h \
    acm_resampler.h \
    acm_speex.h \
    audio_coding_module_impl.h \
    call_statistics.h \
    initial_delay_manager.h \
    nack.h

OTHER_FILES += \
    audio_coding_module.gypi
