#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T15:09:25
#
#-------------------------------------------------

QT       -= core gui

TARGET = webrtc_video_coding
TEMPLATE = lib
CONFIG += staticlib

include (../../webrtc_common.pri)

SOURCES += \
    main/source/codec_database.cc \
    main/source/codec_timer.cc \
    main/source/content_metrics_processing.cc \
    main/source/decoding_state.cc \
    main/source/encoded_frame.cc \
    main/source/frame_buffer.cc \
    main/source/generic_decoder.cc \
    main/source/generic_encoder.cc \
    main/source/inter_frame_delay.cc \
    main/source/jitter_buffer.cc \
    main/source/jitter_estimator.cc \
    main/source/media_opt_util.cc \
    main/source/media_optimization.cc \
    main/source/packet.cc \
    main/source/qm_select.cc \
    main/source/receiver.cc \
    main/source/rtt_filter.cc \
    main/source/session_info.cc \
    main/source/timestamp_extrapolator.cc \
    main/source/timestamp_map.cc \
    main/source/timing.cc \
    main/source/video_coding_impl.cc \
    main/source/video_receiver.cc \
    main/source/video_sender.cc

HEADERS += \
    main/source/codec_database.h \
    main/source/codec_timer.h \
    main/source/content_metrics_processing.h \
    main/source/decoding_state.h \
    main/source/encoded_frame.h \
    main/source/er_tables_xor.h \
    main/source/fec_tables_xor.h \
    main/source/frame_buffer.h \
    main/source/generic_decoder.h \
    main/source/generic_encoder.h \
    main/source/inter_frame_delay.h \
    main/source/internal_defines.h \
    main/source/jitter_buffer_common.h \
    main/source/jitter_buffer.h \
    main/source/jitter_estimator.h \
    main/source/media_opt_util.h \
    main/source/media_optimization.h \
    main/source/nack_fec_tables.h \
    main/source/packet.h \
    main/source/qm_select_data.h \
    main/source/qm_select.h \
    main/source/receiver.h \
    main/source/rtt_filter.h \
    main/source/session_info.h \
    main/source/timestamp_extrapolator.h \
    main/source/timestamp_map.h \
    main/source/timing.h \
    main/source/video_coding_impl.h

OTHER_FILES += \
    main/source/video_coding.gypi
