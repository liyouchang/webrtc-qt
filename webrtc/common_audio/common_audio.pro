#-------------------------------------------------
#
# Project created by QtCreator 2014-02-22T16:44:37
#
#-------------------------------------------------

QT       -= core gui

TARGET = common_audio
TEMPLATE = lib
CONFIG += staticlib


include (../webrtc_common.pri)



SOURCES += \
    audio_util.cc \
    resampler/sinc_resampler.cc \
    resampler/resampler.cc \
    resampler/push_sinc_resampler.cc \
    resampler/push_resampler.cc \
        'signal_processing/auto_corr_to_refl_coef.c'\
        'signal_processing/auto_correlation.c'\
        'signal_processing/complex_fft.c'\
        'signal_processing/complex_bit_reverse.c'\
        'signal_processing/copy_set_operations.c'\
        'signal_processing/cross_correlation.c'\
        'signal_processing/division_operations.c'\
        'signal_processing/dot_product_with_scale.c'\
        'signal_processing/downsample_fast.c'\
        'signal_processing/energy.c'\
        'signal_processing/filter_ar.c'\
        'signal_processing/filter_ar_fast_q12.c'\
        'signal_processing/filter_ma_fast_q12.c'\
        'signal_processing/get_hanning_window.c'\
        'signal_processing/get_scaling_square.c'\
        'signal_processing/ilbc_specific_functions.c'\
        'signal_processing/levinson_durbin.c'\
        'signal_processing/lpc_to_refl_coef.c'\
        'signal_processing/min_max_operations.c'\
        'signal_processing/randomization_functions.c'\
        'signal_processing/refl_coef_to_lpc.c'\
        'signal_processing/real_fft.c'\
        'signal_processing/resample.c'\
        'signal_processing/resample_48khz.c'\
        'signal_processing/resample_by_2.c'\
        'signal_processing/resample_by_2_internal.c'\
        'signal_processing/resample_by_2_internal.h'\
        'signal_processing/resample_fractional.c'\
        'signal_processing/spl_init.c'\
        'signal_processing/spl_sqrt.c'\
        'signal_processing/spl_sqrt_floor.c'\
        'signal_processing/spl_version.c'\
        'signal_processing/splitting_filter.c'\
        'signal_processing/sqrt_of_one_minus_x_squared.c'\
        'signal_processing/vector_scaling_operations.c'\
        'vad/webrtc_vad.c'\
        'vad/vad_core.c'\
        'vad/vad_filterbank.c'\
        'vad/vad_gmm.c'\
        'vad/vad_sp.c'\


HEADERS += \
    resampler/sinusoidal_linear_chirp_source.h \
    resampler/sinc_resampler.h \
    resampler/push_sinc_resampler.h

OTHER_FILES += \
    common_audio.gyp
