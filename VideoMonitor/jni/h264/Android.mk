LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    	:= h264
LOCAL_SRC_FILES := H264Android.c allcodecs.c bitstream.c cabac.c dsputil.c error_resilience.c \
	golomb.c h264.c h264dspenc.c h264idct.c h264_parser.c h264pred.c imgconvert.c jrevdct.c log.c \
	mem.c mpegvideo.c opt.c parser.c rational.c simple_idct.c svq3.c utils.c vp3dsp.c
	
include $(BUILD_SHARED_LIBRARY)
