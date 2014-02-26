#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T14:40:57
#
#-------------------------------------------------

QT       -= core gui

TARGET = NetEq
TEMPLATE = lib
CONFIG += staticlib

include (../../../webrtc_common.pri)

DEFINES += NETEQ_VOICEENGINE_CODECS SCRATCH

INCLUDEPATH += $$webrtc_root/webrtc/modules/audio_coding/codecs/cng/include \
    $$webrtc_root/webrtc/common_audio/signal_processing/include \
    interface

SOURCES += \
    accelerate.c \
    automode.c \
    bgn_update.c \
    bufstats_decision.c \
    cng_internal.c \
    codec_db.c \
    correlator.c \
    dsp_helpfunctions.c \
    dsp.c \
    dtmf_buffer.c \
    dtmf_tonegen.c \
    expand.c \
    mcu_address_init.c \
    mcu_dsp_common.c \
    mcu_reset.c \
    merge.c \
    min_distortion.c \
    mix_voice_unvoice.c \
    mute_signal.c \
    normal.c \
    packet_buffer.c \
    peak_detection.c \
    preemptive_expand.c \
    random_vector.c \
    recin.c \
    recout.c \
    rtcp.c \
    rtp.c \
    set_fs.c \
    signal_mcu.c \
    split_and_insert.c \
    unmute_signal.c \
    webrtc_neteq.c

HEADERS += \
    automode.h \
    buffer_stats.h \
    codec_db_defines.h \
    codec_db.h \
    delay_logging.h \
    dsp_helpfunctions.h \
    dsp.h \
    dtmf_buffer.h \
    dtmf_tonegen.h \
    mcu_dsp_common.h \
    mcu.h \
    neteq_defines.h \
    neteq_error_codes.h \
    neteq_statistics.h \
    packet_buffer.h \
    rtcp.h \
    rtp.h

OTHER_FILES += \
    neteq.gypi \
    neteq_unittests.isolate \
    OWNERS
