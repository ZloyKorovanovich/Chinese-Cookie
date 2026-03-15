#include "render.h"

#define RES_X (60)
#define RES_Y (20)

struct RenderData {
    HANDLE std_out;
    CHAR_INFO draw_buffer[RES_X * RES_Y];
};

static RenderData s_render_data = (RenderData){0};

/* FIX: set console size for permanent window size as intended */
i32 setupDrawConsole() {
    s_render_data.std_out = GetStdHandle(-11);
    if(!s_render_data.std_out || s_render_data.std_out == INVALID_HANDLE_VALUE) {
        printf("failed to get console handle\n");
        return -1;
    }
    if(!SetConsoleOutputCP(CP_UTF8)) {
        printf("failed to set console to utf-8\n");
        return -3;
    }
    return 0;
}

void clearDrawBuffer(const CHAR_INFO *value) {
    for(u32 x = 0; x < RES_X; x++) {
        for(u32 y = 0; y < RES_Y; y++) {
            s_render_data.draw_buffer[x + y * RES_X] = *value;
        }
    }
}

void drawSprite(const Sprite *sprite) {
    CHAR_INFO *       dst_buffer = s_render_data.draw_buffer;
    const CHAR_INFO * src_buffer = sprite->char_infos;
    const u32         size_x     = sprite->x_end - sprite->x_begin;
    const u32         size_y     = sprite->y_end - sprite->y_begin;

    /* 1 - y on y because orientation is changed for user, begin becomes end, end becomes begin */
    const u32         x_begin    = CLAMP(0, RES_X, sprite->x_begin);
    const u32         y_begin    = CLAMP(0, RES_Y, RES_Y - sprite->y_end);
    const u32         x_end      = CLAMP(0, RES_X, sprite->x_end);
    const u32         y_end      = CLAMP(0, RES_Y, RES_Y - sprite->y_begin);

    /* noting to draw case */
    if(x_begin == x_end || y_begin == y_end) {
        return;
    }

    for(u32 x = x_begin, x1 = 0; x != x_end; x++, x1++) {
        for(u32 y = y_begin, y1 = 0; y != y_end; y++, y1++) {
            /* if char src char is transparent (ascii \0), dont change, otherwise set new char */
            CHAR_INFO src_char = src_buffer[x1 + y1 * size_x];
            CHAR_INFO dst_char = dst_buffer[x + y * RES_X];
            dst_char.Attributes |= dst_char.Attributes;
            dst_buffer[x + y * RES_X] = (src_char.Char.AsciiChar == '\0') ? dst_char : src_char;
        }
    }
}

i32 showDrawBuffer() {
    const SMALL_RECT small_rect = {0, 0, RES_X, RES_Y};

    if(!WriteConsoleOutputW(
        s_render_data.std_out, 
        s_render_data.draw_buffer, 
        (COORD){RES_X, RES_Y}, 
        (COORD){0, 0}, 
        (PSMALL_RECT)&small_rect
    )) {
        return -1;
    }
    return 0;
}
