#include "base.h"
#include "render/render.h"

i32 main(i32 argc, char **argv) {
    if(setupDrawConsole() != 0) {
        printf("failed to set up draw console\n");
        return -1;
    }

    CHAR_INFO clear_color = {
        .Char = {.AsciiChar = ' '},
        .Attributes = BACKGROUND_GREEN
    };

    CHAR_INFO test_sprite_buffer[] = {
        (CHAR_INFO) {
            .Char = {.UnicodeChar = L'$'},
            .Attributes = FOREGROUND_BLUE
        },
        (CHAR_INFO) {
            .Char = {.UnicodeChar = L'&'},
            .Attributes = FOREGROUND_BLUE
        },
        (CHAR_INFO) {
            .Char = {.UnicodeChar = L'%'},
            .Attributes = FOREGROUND_BLUE
        },
        (CHAR_INFO) {
            .Char = {.UnicodeChar = L'@'},
            .Attributes = FOREGROUND_BLUE
        }
    };
    Sprite test_sprite = {
        .x_begin = 0,
        .y_begin = 0,
        .x_end = 2,
        .y_end = 2,
        .char_infos = test_sprite_buffer
    };

    while (1) {
        clearDrawBuffer(&clear_color);
        drawSprite(&test_sprite);
        if(showDrawBuffer() != 0) {
            printf("failed to show draw buffer\n");
            return -2;
        }
    }
    
    return 0;
}
