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
                       TextureCache.cpp \
                       ImageLoader.cpp \
                       GraphicsSprite.cpp \
                       gl_code.cpp \
                       Resource.cpp \
                       math/Mat4.cpp \
                       math/MathUtil.cpp \
                       math/Quaternion.cpp \
                       math/Vec2.cpp \
                       math/Vec3.cpp \
                       math/Vec4.cpp \
                       renderer/Shader.cpp \
                       renderer/TextureShader.cpp \
                       renderer/BasicShader.cpp \
                       GraphicsService.cpp \


LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv2

include $(BUILD_SHARED_LIBRARY)
