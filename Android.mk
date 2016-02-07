LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libminizip
LOCAL_SRC_FILES := ioapi.c ioapi_buf.c ioandroid.c unzip.c zip.c
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
include $(BUILD_STATIC_LIBRARY)
