#include <jni.h>
#include "LogHelper.h"

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM * vm, void * reserved)
{
    LOGE("JNI_OnLoad");
    return JNI_VERSION_1_4;
}

JNIEXPORT void JNI_OnUnload(JavaVM *jvm, void *reserved)
{
    LOGE("JNI_OnUnload");
}
