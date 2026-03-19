#ifndef _BASE_INCLUDED
#define _BASE_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

typedef char i8;
typedef short i16;
typedef int i32;
typedef long i32x;
typedef long long i64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned u32;
typedef unsigned long u32x;
typedef unsigned long long u64;

typedef float f32;
typedef double f64;

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)
#define CLAMP(a, b, t) (t < a ? a : (t > b ? b : t))

#endif
