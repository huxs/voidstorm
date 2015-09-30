#pragma once
#ifdef ANDROID
#include <android/log.h>
#define PRINT(...) __android_log_print(ANDROID_LOG_VERBOSE, _PROJECT_NAME, __VA_ARGS__);
#else
#include <stdio.h>
#define PRINT(...) (printf(__VA_ARGS__))
#endif

