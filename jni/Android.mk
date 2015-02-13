LOCAL_PATH:=$(call my-dir)

include $(CLEAR_VARS)

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
