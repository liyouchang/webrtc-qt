#-------------------------------------------------
#
# Project created by QtCreator 2014-02-24T13:38:02
#
#-------------------------------------------------

QT       -= core gui

TARGET = rtp_rtcp
TEMPLATE = lib
CONFIG += staticlib


include (../../webrtc_common.pri)

SOURCES += \
    source/bitrate.cc \
    source/dtmf_queue.cc \
    source/fec_receiver_impl.cc \
    source/forward_error_correction_internal.cc \
    source/forward_error_correction.cc \
    source/producer_fec.cc \
    source/receive_statistics_impl.cc \
    source/rtcp_receiver_help.cc \
    source/rtcp_receiver.cc \
    source/rtcp_sender.cc \
    source/rtcp_utility.cc \
    source/rtp_format_vp8.cc \
    source/rtp_header_extension.cc \
    source/rtp_header_parser.cc \
    source/rtp_packet_history.cc \
    source/rtp_payload_registry.cc \
    source/rtp_receiver_audio.cc \
    source/rtp_receiver_impl.cc \
    source/rtp_receiver_strategy.cc \
    source/rtp_receiver_video.cc \
    source/rtp_rtcp_impl.cc \
    source/rtp_sender_audio.cc \
    source/rtp_sender_video.cc \
    source/rtp_sender.cc \
    source/rtp_utility.cc \
    source/ssrc_database.cc \
    source/tmmbr_help.cc \
    source/vp8_partition_aggregator.cc

HEADERS += \
    source/bitrate.h \
    source/byte_io.h \
    source/dtmf_queue.h \
    source/fec_private_tables_bursty.h \
    source/fec_private_tables_random.h \
    source/fec_receiver_impl.h \
    source/forward_error_correction_internal.h \
    source/forward_error_correction.h \
    source/producer_fec.h \
    source/receive_statistics_impl.h \
    source/rtcp_receiver_help.h \
    source/rtcp_receiver.h \
    source/rtcp_sender.h \
    source/rtcp_utility.h \
    source/rtp_format_video_generic.h \
    source/rtp_format_vp8.h \
    source/rtp_header_extension.h \
    source/rtp_packet_history.h \
    source/rtp_receiver_audio.h \
    source/rtp_receiver_impl.h \
    source/rtp_receiver_strategy.h \
    source/rtp_receiver_video.h \
    source/rtp_rtcp_config.h \
    source/rtp_rtcp_impl.h \
    source/rtp_sender_audio.h \
    source/rtp_sender_video.h \
    source/rtp_sender.h \
    source/rtp_utility.h \
    source/ssrc_database.h \
    source/tmmbr_help.h \
    source/video_codec_information.h \
    source/vp8_partition_aggregator.h

OTHER_FILES += \
    source/rtp_rtcp.gypi
