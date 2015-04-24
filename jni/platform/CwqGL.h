#ifndef CWQGL_H
#define CWQGL_H

#if defined(ANDROID) || defined(__ANDROID__)

#include <GLES2/gl2.h>

#else // !ANDROID !__ANDROID__

#if defined(_WIN32) || defined(__WIN32__)

#include <GL/glew.h>

#endif // !_WIN32 !__WIN32__

#endif // !ANDROID !__ANDROID__

#endif // !CWQGL_H
