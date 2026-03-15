#include "../base.h"

struct Sprite {
    u32 x_begin;
    u32 y_begin;
    u32 x_end;
    u32 y_end;
    const CHAR_INFO *char_infos;
};

i32 setupDrawConsole();
void clearDrawBuffer(const CHAR_INFO *value);
void drawSprite(const Sprite *sprite);
i32 showDrawBuffer();
