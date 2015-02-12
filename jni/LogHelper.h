#ifndef LOGHELPER_H
#define LOGHELPER_H

#define DEBUG_NATVIE

#ifdef DEBUG_NATVIE

#if defined(ANDROID) || defined(__ANDROID__)

#include <android/log.h>
#define  LOG_TAG    "CwqEngine"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#else // !ANDROID !__ANDROID__

#ifdef WIN32

#include <stdio.h>
#define  LOGI(...)  printf("INFO: ");printf(__VA_ARGS__);printf("\n")
#define  LOGV(...)  printf("VERBOSE: ");printf(__VA_ARGS__);printf("\n")
#define  LOGD(...)  printf("DEBUG: ");printf(__VA_ARGS__);printf("\n")
#define  LOGW(...)  printf("WARN: ");printf(__VA_ARGS__);printf("\n")
#define  LOGE(...)  printf("ERROR: ");printf(__VA_ARGS__);printf("\n")

#endif // !WIN32

#endif // !ANDROID !__ANDROID__

#else // !DEBUG_NATVIE

#define  LOGI(...)
#define  LOGV(...)
#define  LOGD(...)
#define  LOGW(...)
#define  LOGE(...)

#endif // !DEBUG_NATVIE

#endif // !LOGHELPER_H
