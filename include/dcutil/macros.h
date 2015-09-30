#pragma once

#include <stdint.h>

#define MAKEFOURCC(a, b, c, d) ( ( (uint32_t)(a) | ( (uint32_t)(b) << 8) | ( (uint32_t)(c) << 16) | ( (uint32_t)(d) << 24) ) )

#define ARRAYSIZE(A) (sizeof(A) / sizeof(A[0]))

#define Kilobytes(Value) ((Value) * 1024)
#define Megabytes(Value) (Kilobytes(Value) * 1024)
#define Gigabytes(Value) (Megabytes(Value) * 1024)
#define Terabytes(Value) (Gigabytes(Value) * 1024)

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
