#-------------------------------------------------
#
# Project created by QtCreator 2014-02-22T14:16:54
#
#-------------------------------------------------

QT       -= core gui

TARGET = video_engine_core
TEMPLATE = lib
CONFIG += staticlib



include (../webrtc_common.pri)




SOURCES += \
        'call_stats.cc'\
        'encoder_state_feedback.cc'\
        'overuse_frame_detector.cc'\
        'stream_synchronization.cc'\
        'vie_base_impl.cc'\
        'vie_capture_impl.cc'\
        'vie_codec_impl.cc'\
        'vie_external_codec_impl.cc'\
        'vie_image_process_impl.cc'\
        'vie_impl.cc'\
        'vie_network_impl.cc'\
        'vie_ref_count.cc'\
        'vie_render_impl.cc'\
        'vie_rtp_rtcp_impl.cc'\
        'vie_shared_data.cc'\
        'vie_capturer.cc'\
        'vie_channel.cc'\
        'vie_channel_group.cc'\
        'vie_channel_manager.cc'\
        'vie_encoder.cc'\
        'vie_file_image.cc'\
        'vie_frame_provider_base.cc'\
        'vie_input_manager.cc'\
        'vie_manager_base.cc'\
        'vie_receiver.cc'\
        'vie_remb.cc'\
        'vie_renderer.cc'\
        'vie_render_manager.cc'\
        'vie_sender.cc'\
        'vie_sync_module.cc'\



HEADERS += \
        'include/vie_base.h'\
        'include/vie_capture.h'\
        'include/vie_codec.h'\
        'include/vie_errors.h'\
        'include/vie_external_codec.h'\
        'include/vie_image_process.h'\
        'include/vie_network.h'\
        'include/vie_render.h'\
        'include/vie_rtp_rtcp.h'\
        'call_stats.h'\
        'encoder_state_feedback.h'\
        'overuse_frame_detector.h'\
        'stream_synchronization.h'\
        'vie_base_impl.h'\
        'vie_capture_impl.h'\
        'vie_codec_impl.h'\
        'vie_defines.h'\
        'vie_external_codec_impl.h'\
        'vie_image_process_impl.h'\
        'vie_impl.h'\
        'vie_network_impl.h'\
        'vie_ref_count.h'\
        'vie_remb.h'\
        'vie_render_impl.h'\
        'vie_rtp_rtcp_impl.h'\
        'vie_shared_data.h'\
        'vie_capturer.h'\
        'vie_channel.h'\
        'vie_channel_group.h'\
        'vie_channel_manager.h'\
        'vie_encoder.h'\
        'vie_file_image.h'\
        'vie_frame_provider_base.h'\
        'vie_input_manager.h'\
        'vie_manager_base.h'\
        'vie_receiver.h'\
        'vie_renderer.h'\
        'vie_render_manager.h'\
        'vie_sender.h'\
        'vie_sync_module.h'\

