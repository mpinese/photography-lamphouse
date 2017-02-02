#pragma once

#ifdef _DEBUG

#include <WProgram.h>

void DEBUG_PRINT(const char* str)
{
    Serial.print(millis()); \
    Serial.print(": "); \
    Serial.print(__FUNCTION__); \
    Serial.print("() in "); \
    Serial.print(__FILE__); \
    Serial.print(':'); \
    Serial.print(__LINE__); \
    Serial.print(' '); \
    Serial.print(str);
}

void DEBUG_PRINTF(const char *format, ...)
{
    char buf[128];
    va_list args;

    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    buf[sizeof(buf) / sizeof(buf[0]) - 1] = '\0';

    DEBUG_PRINT(buf);
}

#else

#define DEBUG_PRINT(str)
#define DEBUG_PRINTF(...)

#endif
