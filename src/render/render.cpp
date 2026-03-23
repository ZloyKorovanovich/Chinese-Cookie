#include "render.h"

typedef struct {
    CHAR16 frame_buffer[RESOLUTION_X * RESOLUTION_Y];
    UINTN  color_buffer[RESOLUTION_X * RESOLUTION_Y];
} RenderCtx;

static RenderCtx s_render_ctx = (RenderCtx){0};

void clearFrameBuffer(
    CHAR16 value, 
    UINTN  color
) {
    CHAR16* char_buffer  = s_render_ctx.frame_buffer;
    UINTN*  color_buffer = s_render_ctx.color_buffer;
    UINT32  x            = 0;
    UINT32  y            = 0;

    for(y = 0; y != RESOLUTION_Y; y++) {
        for(x = 0; x != RESOLUTION_X; x++) {
            char_buffer [x + y * RESOLUTION_X] = value;
            color_buffer[x + y * RESOLUTION_X] = color;
        }
    }
}

void drawSprite(
    const Sprite* sprite,
    BlendingMode  blending_mode
) {
    const CHAR16* src_buffer       = sprite->buffer;
    UINTN         src_color        = sprite->color;

    CHAR16*       dst_buffer       = s_render_ctx.frame_buffer;
    UINTN*        dst_color_buffer = s_render_ctx.color_buffer;
    INT32         x_begin          = sprite->x;
    INT32         y_begin          = sprite->y;
    INT32         x_end            = sprite->x + sprite->size_x;
    INT32         y_end            = sprite->y + sprite->size_y;
    INT32         x_size           = ABS(sprite->size_x);
    INT32         y_size           = ABS(sprite->size_y);
    INT32         x_src            = 0;
    INT32         y_src            = 0;
    INT32         adj_x            = 0;
    INT32         adj_y            = 0;
    INT32         x                = 0;
    INT32         y                = 0;
    CHAR16        src_char         = 0;
    CHAR16        dst_char         = 0;

    x_begin = CLAMP(0, RESOLUTION_X, x_begin);
    y_begin = CLAMP(0, RESOLUTION_Y, y_begin);
    x_end   = CLAMP(0, RESOLUTION_X, x_end);
    y_end   = CLAMP(0, RESOLUTION_Y, y_end);

    if(x_begin == x_end || y_begin == y_end) {
        return;
    }

    adj_x = (x_begin < x_end) ? 1 : -1;
    adj_y = (y_begin < y_end) ? 1 : -1;

    for(y = y_begin, y_src = 0; y != y_end; y += adj_y, y_src++) {
        for(x = x_begin, x_src = 0; x != x_end; x+= adj_x, x_src++) {
            dst_char = dst_buffer[x + y * RESOLUTION_X];
            src_char = src_buffer[x_src + y_src * x_size];
            
            dst_buffer      [x + y * RESOLUTION_X] = (src_char == L'+') ? dst_char : src_char;
            if(src_char != 0) {
                if(blending_mode == BM_OVERWRITE) {
                    dst_color_buffer[x + y * RESOLUTION_X] = src_color;
                }
                if(blending_mode == BM_OR) {
                    dst_color_buffer[x + y * RESOLUTION_X] |= src_color;
                }
                if(blending_mode == BM_XOR) {
                    dst_color_buffer[x + y * RESOLUTION_X] ^= src_color;
                }
                if(blending_mode == BM_ADD) {
                    dst_color_buffer[x + y * RESOLUTION_X] += src_color;
                }
            }
        }
    }
}

void showFrameBuffer(
    SIMPLE_TEXT_OUTPUT_INTERFACE* text_output
) {
    const CHAR16* char_buffer  = s_render_ctx.frame_buffer;
    const UINTN*  color_buffer = s_render_ctx.color_buffer;

    CHAR16 temp_str[128] = {0};
    UINTN  color_mode    = 0;
    UINT32 mode_x        = 0;
    UINT32 x             = 0;
    UINT32 y             = 0;
    
    text_output->SetCursorPosition(text_output, 0, 0);

    for(y = 0; y != RESOLUTION_Y; y++) {
        color_mode = color_buffer[y * RESOLUTION_X];
        mode_x = 0;
        for(x = 0; x != RESOLUTION_X; x++) {
            if(color_buffer[x + y * RESOLUTION_X] != color_mode || mode_x == 128) {
                temp_str[mode_x] = 0;
                text_output->SetAttribute(text_output, color_mode);
                text_output->OutputString(text_output, temp_str);

                color_mode = color_buffer[x + y * RESOLUTION_X];
                mode_x     = 0;
            } 
            temp_str[mode_x++] = char_buffer[x + y * RESOLUTION_X];
        }
        if(mode_x != 0) {
            temp_str[mode_x] = 0;
            text_output->SetAttribute(text_output, color_mode);
            text_output->OutputString(text_output, temp_str);
        }

        text_output->OutputString(text_output, (CHAR16*)L"\r\n");
    }
}
