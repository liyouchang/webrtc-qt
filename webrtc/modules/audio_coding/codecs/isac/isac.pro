#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T14:25:11
#
#-------------------------------------------------

QT       -= core gui

TARGET = iSAC
TEMPLATE = lib
CONFIG += staticlib

include (../../../../webrtc_common.pri)

INCLUDEPATH += main/interface \
    $$webrtc_root/webrtc/common_audio/signal_processing/include

SOURCES += \
    main/source/arith_routines_hist.c \
    main/source/arith_routines_logist.c \
    main/source/arith_routines.c \
    main/source/bandwidth_estimator.c \
    main/source/crc.c \
    main/source/decode_bwe.c \
    main/source/decode.c \
    main/source/encode_lpc_swb.c \
    main/source/encode.c \
    main/source/entropy_coding.c \
    main/source/fft.c \
    main/source/filter_functions.c \
    main/source/filterbank_tables.c \
    main/source/filterbanks.c \
    main/source/intialize.c \
    main/source/isac.c \
    main/source/lattice.c \
    main/source/lpc_analysis.c \
    main/source/lpc_gain_swb_tables.c \
    main/source/lpc_shape_swb12_tables.c \
    main/source/lpc_shape_swb16_tables.c \
    main/source/lpc_tables.c \
    main/source/pitch_estimator.c \
    main/source/pitch_filter.c \
    main/source/pitch_gain_tables.c \
    main/source/pitch_lag_tables.c \
    main/source/spectrum_ar_model_tables.c \
    main/source/transform.c

HEADERS += \
    main/source/arith_routines.h \
    main/source/bandwidth_estimator.h \
    main/source/codec.h \
    main/source/crc.h \
    main/source/encode_lpc_swb.h \
    main/source/entropy_coding.h \
    main/source/fft.h \
    main/source/filterbank_tables.h \
    main/source/lpc_analysis.h \
    main/source/lpc_gain_swb_tables.h \
    main/source/lpc_shape_swb12_tables.h \
    main/source/lpc_shape_swb16_tables.h \
    main/source/lpc_tables.h \
    main/source/os_specific_inline.h \
    main/source/pitch_estimator.h \
    main/source/pitch_gain_tables.h \
    main/source/pitch_lag_tables.h \
    main/source/settings.h \
    main/source/spectrum_ar_model_tables.h \
    main/source/structs.h

OTHER_FILES += \
    main/source/isac.gypi \
    fix/source/isacfix.gypi
