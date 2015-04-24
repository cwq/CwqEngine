#ifndef CROSS_SLEEP_H
#define CROSS_SLEEP_H

#if defined(_WIN32) || defined(__WIN32__)

#include <windows.h>
void sleep_ms(int ms)
{
    Sleep(ms);
}

#else

#include <unistd.h>
void sleep_ms(int ms)
{
    usleep(ms*1000);
}

#endif

#endif // !CROSS_SLEEP_H
