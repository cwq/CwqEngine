LOCAL_PATH:=$(call my-dir)

include $(CLEAR_VARS)

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    LOCAL_CFLAGS += -DHAVE_NEON=1
    LOCAL_ARM_NEON  := true 
    CFLAGS += -mfpu=neon 
endif

LOCAL_MODULE        := cwqengine

LOCAL_SRC_FILES     := CwqEngineJNI.cpp \
                       CwqEngine.cpp \
                       CwqEngineJNIHelper.cpp \
                       TestJNI.cpp \
                       Image.cpp \
                       Texture2D.cpp \
                       ImageLoader.cpp


LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv2

include $(BUILD_SHARED_LIBRARY)
