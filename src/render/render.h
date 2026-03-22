#ifndef _RENDER_INCLUDED
#define _RENDER_INCLUDED

#include "../base.h"

#define RESOLUTION_X   (60)
#define RESOLUTION_Y   (20)

typedef enum {
    BM_OVERWRITE,
    BM_OR,
    BM_XOR,
    BM_ADD
} BlendingMode;

typedef struct {
    CHAR16* buffer;
    UINTN   color;
    INT32   x;
    INT32   y;
    INT32   size_x;
    INT32   size_y;
} Sprite;

void clearFrameBuffer(CHAR16 value, UINTN color);
void drawSprite(const Sprite* sprite, BlendingMode blending_mode);
void showFrameBuffer(SIMPLE_TEXT_OUTPUT_INTERFACE* text_output);

#endif
