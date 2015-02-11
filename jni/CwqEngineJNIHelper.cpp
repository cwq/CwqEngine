#include "CwqEngineJNIHelper.h"
#include <pthread.h>
#include <assert.h>
#include "LogHelper.h"

#define JNI_CLASS_HANDLER "com/cwq/cwqengine/CwqEngineHandler"

#define CHECK_RET(condition__, retval__, ...) \
    if (!(condition__)) { \
        LOGE(__VA_ARGS__); \
        return (retval__); \
    }

static pthread_key_t current_jni_env;

JavaVM* CwqEngineJNIHelper::mJavaVM = NULL;
jclass CwqEngineJNIHelper::jHandler = NULL;
jmethodID CwqEngineJNIHelper::postEventFromNative = NULL;

void CwqEngineJNIHelper::setJavaVM(JavaVM *javaVM)
{
    LOGE("setJavaVM thread %ld", pthread_self());
    mJavaVM = javaVM;
    pthread_key_create(&current_jni_env, CwqEngineJNIHelper::detachCurrentEnv);
}

JavaVM* CwqEngineJNIHelper::getJavaVM()
{
    LOGE("getJavaVM thread %ld", pthread_self());
    return mJavaVM;
}

JNIEnv* CwqEngineJNIHelper::attachCurrentEnv(JavaVM* jvm)
{
    JNIEnv *env;
    JavaVMAttachArgs args;

    LOGE("Attaching thread %ld", pthread_self());
    args.version = JNI_VERSION_1_4;
    args.name = NULL;
    args.group = NULL;

    if (jvm->AttachCurrentThread(&env, &args) < 0) {
        LOGE("Failed to attach current thread");
        return NULL;
    }

    return env;
}

void CwqEngineJNIHelper::detachCurrentEnv(void *env)
{
    LOGE("Detaching thread %ld", pthread_self());
    mJavaVM->DetachCurrentThread();
}

JNIEnv* CwqEngineJNIHelper::getEnv()
{
    JNIEnv *env;

    if ((env = (JNIEnv*)pthread_getspecific(current_jni_env)) == NULL) {
        env = CwqEngineJNIHelper::attachCurrentEnv(mJavaVM);
        pthread_setspecific(current_jni_env, env);
    }

    return env;
}

void CwqEngineJNIHelper::setJHandlerClass(jclass handler)
{
    jHandler = handler;
}

jclass CwqEngineJNIHelper::getJHandlerClass()
{
    return jHandler;
}

void CwqEngineJNIHelper::setPostEventFromNativeID(jmethodID jmethodID)
{
    postEventFromNative = jmethodID;
}

jmethodID CwqEngineJNIHelper::getPostEventFromNativeID()
{
    return postEventFromNative;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM * vm, void * reserved)
{
    LOGE("JNI_OnLoad");
    JNIEnv* env = NULL;
    jclass clazz = NULL;
    if ((vm)->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
    {
        return -1;
    }

    CwqEngineJNIHelper::setJavaVM(vm);

    assert(env != NULL);

    clazz = (env)->FindClass(JNI_CLASS_HANDLER);
    CHECK_RET(clazz, -1, "missing %s", JNI_CLASS_HANDLER);

    // FindClass returns LocalReference
    CwqEngineJNIHelper::setJHandlerClass((jclass)(env)->NewGlobalRef(clazz));
    CHECK_RET(CwqEngineJNIHelper::getJHandlerClass(), -1, "%s NewGlobalRef failed", JNI_CLASS_HANDLER);
    (env)->DeleteLocalRef(clazz);

    CwqEngineJNIHelper::setPostEventFromNativeID((env)->GetStaticMethodID(CwqEngineJNIHelper::getJHandlerClass(),
            "postEventFromNative", "(Ljava/lang/Object;III)V"));
    CHECK_RET(CwqEngineJNIHelper::getPostEventFromNativeID(), -1, "missing postEventFromNative");

    return JNI_VERSION_1_4;
}

JNIEXPORT void JNI_OnUnload(JavaVM *jvm, void *reserved)
{
    LOGE("JNI_OnUnload");
    JNIEnv* env = NULL;
    if ((jvm)->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        return;
    }

    assert(env != NULL);

    (env)->DeleteGlobalRef(CwqEngineJNIHelper::getJHandlerClass());
}
