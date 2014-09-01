LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

MY_MODULE_DIR 		:= p2p
LOCAL_MODULE    	:= $(MY_MODULE_DIR)

P2P_LIB_PATH := $(CURDIR)/../out/android/libs
LOCAL_LDLIBS := -llog  -landroid  \
	$(P2P_LIB_PATH)/libjingle_app.a \
	$(P2P_LIB_PATH)/libjingle_p2p.a \
	$(P2P_LIB_PATH)/libjingle.a \
 	$(P2P_LIB_PATH)/libjsoncpp.a 
 		
LOCAL_CFLAGS := -DPOSIX  

LOCAL_C_INCLUDES := $(CURDIR)/..
LOCAL_SRC_FILES := JniPeerConnection.cpp JniUtil.cpp KeJniTunnelClient.cpp VideoPhone.cpp

include $(BUILD_SHARED_LIBRARY)