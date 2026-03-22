#ifndef _BASE_INCLUDED
#define _BASE_INCLUDED

#include <efi.h>

#define CLAMP(a, b, t) (t < a ? a : (t > b ? b : t))
#define MIN(a, b)      (a < b ? a : b)
#define MAX(a, b)      (a > b ? a : b)
#define ABS(a)         (a < 0 ? -a : a)

typedef UINT8 BYTE;

#endif
