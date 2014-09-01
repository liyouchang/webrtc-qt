LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    	:= p2p
LOCAL_SRC_FILES := $(CURDIR)/../out/android/zmqclient/libzmqclient.so

include $(BUILD_SHARED_LIBRARY)
