#include <windows.h>

int main(int argc, char **argv) {
    /* получаем handle output консоли */
    HANDLE console = GetStdHandle(-11);
    /* пытаемся задать размер консоли */
    SetConsoleScreenBufferSize(console, (COORD){60, 24});
    /* задаем формат для поддрежки UTF-8 */
    SetConsoleOutputCP(CP_UTF8);

    /* создаем массив буковок */
    CHAR_INFO chars[1024 * 64];
    /* записываем в массив букв икроглифы */
    for(int i = 0; i < 1024 * 64; i++) {
        chars[i] = (CHAR_INFO) {
            .Char = { .UnicodeChar = L'木' }, /* unicode символ */
            .Attributes = BACKGROUND_GREEN | FOREGROUND_GREEN | FOREGROUND_INTENSITY /* аттрибуты, цвет */
        };
    }

    /* это регион отрисовки, короче еще одна хрень от виндоуса*/
    SMALL_RECT small_rect = {0, 0, 60, 24};
    /* бесконечный цикл */
    while(true) {
        /* рисуем прямоугольником в консоль до бесконечности */
        if(!WriteConsoleOutputW(console, chars, (COORD){60, 24}, (COORD){0, 0}, &small_rect)) {
            return -4;
        }
    }

    return 0;
}
