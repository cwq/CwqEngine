LOCAL_PATH:=$(call my-dir)

include $(CLEAR_VARS)

MEDIA_PLAYER_PATH := mediaplayer
MEDIA_PLAYER_LOCAL_PATH := $(LOCAL_PATH)/$(MEDIA_PLAYER_PATH)

LOCAL_MODULE := ffmpeg
LOCAL_SRC_FILES := $(MEDIA_PLAYER_PATH)/libs/libffmpeg.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_CFLAGS := -D__STDC_CONSTANT_MACROS
FFMPEG_PATH := $(MEDIA_PLAYER_LOCAL_PATH)/ffmpeg/include

LS_CPP=$(subst $(LOCAL_PATH)/,,$(wildcard $(1)/*.cpp))
LS_C=$(subst $(LOCAL_PATH)/,,$(wildcard $(1)/*.c))

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    LOCAL_CFLAGS += -DHAVE_NEON=1
    LOCAL_ARM_NEON  := true 
    CFLAGS += -mfpu=neon 
endif

LOCAL_C_INCLUDES := $(MEDIA_PLAYER_LOCAL_PATH)/src \
                    $(FFMPEG_PATH)  \

LOCAL_MODULE        := cwqengine

LOCAL_SRC_FILES     := $(call LS_CPP, $(LOCAL_PATH)/base) \
                       $(call LS_CPP, $(LOCAL_PATH)/math) \
                       $(call LS_CPP, $(LOCAL_PATH)/engine) \
                       $(call LS_CPP, $(LOCAL_PATH)/renderer) \
                       $(call LS_CPP, $(LOCAL_PATH)/platform) \
                       $(call LS_CPP, $(MEDIA_PLAYER_LOCAL_PATH)/src/media) \
                       $(call LS_CPP, $(MEDIA_PLAYER_LOCAL_PATH)/src/audio) \
                       $(call LS_CPP, $(MEDIA_PLAYER_LOCAL_PATH)/src/video) \
                       $(call LS_CPP, $(MEDIA_PLAYER_LOCAL_PATH)/src/ijkutil) \
                       $(call LS_CPP, $(MEDIA_PLAYER_LOCAL_PATH)/src/mediaplayer) \
                       test/TestJNI.cpp \


LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv2

LOCAL_SHARED_LIBRARIES := ffmpeg

include $(BUILD_SHARED_LIBRARY)
