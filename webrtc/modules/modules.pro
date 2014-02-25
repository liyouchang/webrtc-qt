TEMPLATE = subdirs

SUBDIRS += \
    video_capture \
    audio_coding \
    audio_coding/main/acm2 \
    video_render \
    audio_device \
    audio_processing \
    utility/webrtc_utility.pro \
    media_file \
    rtp_rtcp \
    audio_conference_mixer \
    audio_coding/codecs/audio_codecs.pro \
    audio_coding/neteq4 \
    audio_coding/neteq \
    video_coding \
    video_processing \
    pacing \
    remote_bitrate_estimator \
    video_coding/utility/video_coding_utility.pro \
    bitrate_controller \
    video_coding/codecs/i420
