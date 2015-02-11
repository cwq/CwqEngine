#ifndef CWQENGINEJNIHELPER_H
#define CWQENGINEJNIHELPER_H
#include <jni.h>

class CwqEngineJNIHelper
{
public:
    static void setJavaVM(JavaVM *javaVM);
    static JavaVM* getJavaVM();

    static JNIEnv* getEnv();

    static void setJHandlerClass(jclass handler);
    static jclass getJHandlerClass();

    static void setPostEventFromNativeID(jmethodID jmethodID);
    static jmethodID getPostEventFromNativeID();

    static void postEventToJava(jobject weak_this, int what, int arg1, int arg2);

private:
    static JavaVM* mJavaVM;

    static JNIEnv* attachCurrentEnv(JavaVM* jvm);
    static void detachCurrentEnv(void *env);

    static jclass jHandlerClass;
    static jmethodID postEventFromNative;
};

#endif // !CWQENGINEJNIHELPER_H
