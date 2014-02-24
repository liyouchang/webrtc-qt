#-------------------------------------------------
#
# Project created by QtCreator 2014-02-22T15:27:29
#
#-------------------------------------------------

QT       -= core gui

TARGET = voice_engine
TEMPLATE = lib
CONFIG += staticlib



include (../webrtc_common.pri)

DEFINES += NOMINMAX
#INCLUDEPATH += ../system_wrappers/interface

#INCLUDEPATH += $$PWD/../../third_party/libyuv/include

SOURCES += \
        'channel.cc'\
        'channel_manager.cc'\
        'dtmf_inband.cc'\
        'dtmf_inband_queue.cc'\
        'level_indicator.cc'\
        'monitor_module.cc'\
        'output_mixer.cc'\
        'output_mixer_internal.cc'\
        'shared_data.cc'\
        'statistics.cc'\
        'transmit_mixer.cc'\
        'utility.cc'\
        'voe_audio_processing_impl.cc'\
        'voe_base_impl.cc'\
        'voe_call_report_impl.cc'\
        'voe_codec_impl.cc'\
        'voe_dtmf_impl.cc'\
        'voe_encryption_impl.cc'\
        'voe_external_media_impl.cc'\
        'voe_file_impl.cc'\
        'voe_hardware_impl.cc'\
        'voe_neteq_stats_impl.cc'\
        'voe_network_impl.cc'\
        'voe_rtp_rtcp_impl.cc'\
        'voe_video_sync_impl.cc'\
        'voe_volume_control_impl.cc'\
        'voice_engine_impl.cc'\



HEADERS += \
        '../common_types.h'\
        '../engine_configurations.h'\
        '../typedefs.h'\
        'include/voe_audio_processing.h'\
        'include/voe_base.h'\
        'include/voe_call_report.h'\
        'include/voe_codec.h'\
        'include/voe_dtmf.h'\
        'include/voe_encryption.h'\
        'include/voe_errors.h'\
        'include/voe_external_media.h'\
        'include/voe_file.h'\
        'include/voe_hardware.h'\
        'include/voe_neteq_stats.h'\
        'include/voe_network.h'\
        'include/voe_rtp_rtcp.h'\
        'include/voe_video_sync.h'\
        'include/voe_volume_control.h'\
        'channel.h'\
        'voice_engine_impl.h'\
        'voe_volume_control_impl.h'\
        'voice_engine_defines.h'\
        'voe_video_sync_impl.h'\
        'voe_rtp_rtcp_impl.h'\
        'voe_network_impl.h'\
        'voe_neteq_stats_impl.h'\
        'voe_hardware_impl.h'\
        'voe_file_impl.h'\
        'voe_encryption_impl.h'\
        'voe_external_media_impl.h'\
        'voe_codec_impl.h'\
        'voe_dtmf_impl.h'\
        'voe_call_report_impl.h'\
        'voe_audio_processing_impl.h'\
        'utility.h'\
        'statistics.h'\
        'transmit_mixer.h'\
        'shared_data.h'\
        'output_mixer_internal.h'\
        'output_mixer.h'\
        'monitor_module.h'\
        'level_indicator.h'\
        'dtmf_inband_queue.h'\
        'channel_manager.h'\
        'voe_base_impl.h'\
        'dtmf_inband.h'\
