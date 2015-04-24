LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libminizip
LOCAL_SRC_FILES := ioapi.c unzip.c zip.c
include $(BUILD_STATIC_LIBRARY)
