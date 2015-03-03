#ifndef BASE_CWQMACROS_H
#define BASE_CWQMACROS_H


#define SAFE_DELETE(p)           do { delete (p); (p) = nullptr; } while(0)
#define SAFE_DELETE_ARRAY(p)     do { if(p) { delete[] (p); (p) = nullptr; } } while(0)
#define SAFE_FREE(p)             do { if(p) { free(p); (p) = nullptr; } } while(0)
#define SAFE_RELEASE(p)          do { if(p) { (p)->release(); } } while(0)
#define SAFE_RELEASE_NULL(p)     do { if(p) { (p)->release(); (p) = nullptr; } } while(0)
#define SAFE_RETAIN(p)           do { if(p) { (p)->retain(); } } while(0)
#define BREAK_IF(cond)           if(cond) break

// Assert macros.
#if !defined(NDK_DEBUG) || NDK_DEBUG == 0
#define GP_ASSERT(expression)
#else
#include <cassert>
#define GP_ASSERT(expression) assert(expression)
#endif

#ifndef FLT_EPSILON
#define FLT_EPSILON     1.192092896e-07F
#endif // FLT_EPSILON

#if !defined(NDK_DEBUG) || NDK_DEBUG == 0
#define CHECK_GL_ERROR_DEBUG()
#else
#define CHECK_GL_ERROR_DEBUG() \
do { \
    GLenum __error = glGetError(); \
        if(__error) { \
            LOGE("OpenGL error 0x%04X in %s %s %d\n", __error, __FILE__, __FUNCTION__, __LINE__); \
    } \
} while (false)
#endif

#endif //!BASE_CWQMACROS_H
