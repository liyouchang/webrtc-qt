#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T08:49:38
#
#-------------------------------------------------

QT       -= core gui

TARGET = audio_coding_module
TEMPLATE = lib
CONFIG += staticlib

include (../../webrtc_common.pri)

INCLUDEPATH += main/interface ../interface

SOURCES += \
    main/source/acm_amr.cc \
    main/source/acm_amrwb.cc \
    main/source/acm_celt.cc \
    main/source/acm_cng.cc \
    main/source/acm_codec_database.cc \
    main/source/acm_dtmf_detection.cc \
    main/source/acm_dtmf_playout.cc \
    main/source/acm_g722.cc \
    main/source/acm_g729.cc \
    main/source/acm_g7221.cc \
    main/source/acm_g7221c.cc \
    main/source/acm_g7291.cc \
    main/source/acm_generic_codec.cc \
    main/source/acm_gsmfr.cc \
    main/source/acm_ilbc.cc \
    main/source/acm_isac.cc \
    main/source/acm_neteq.cc \
    main/source/acm_opus.cc \
    main/source/acm_pcm16b.cc \
    main/source/acm_pcma.cc \
    main/source/acm_pcmu.cc \
    main/source/acm_red.cc \
    main/source/acm_resampler.cc \
    main/source/acm_speex.cc \
    main/source/audio_coding_module_impl.cc

HEADERS += \
    main/source/acm_amr.h \
    main/source/acm_amrwb.h \
    main/source/acm_celt.h \
    main/source/acm_cng.h \
    main/source/acm_codec_database.h \
    main/source/acm_dtmf_detection.h \
    main/source/acm_dtmf_playout.h \
    main/source/acm_g722.h \
    main/source/acm_g729.h \
    main/source/acm_g7221.h \
    main/source/acm_g7221c.h \
    main/source/acm_g7291.h \
    main/source/acm_generic_codec.h \
    main/source/acm_gsmfr.h \
    main/source/acm_ilbc.h \
    main/source/acm_isac_macros.h \
    main/source/acm_isac.h \
    main/source/acm_neteq.h \
    main/source/acm_opus.h \
    main/source/acm_pcm16b.h \
    main/source/acm_pcma.h \
    main/source/acm_pcmu.h \
    main/source/acm_red.h \
    main/source/acm_resampler.h \
    main/source/acm_speex.h \
    main/source/audio_coding_module_impl.h

OTHER_FILES += \
    main/source/audio_coding_module.gypi \
    main/acm2/audio_coding_module.gypi
