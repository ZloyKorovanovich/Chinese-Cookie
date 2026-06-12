#ifndef CONSOLE_RENDERER_H_
#define CONSOLE_RENDERER_H_

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <string>
#include <vector>

#include "Sprite.h"

class ConsoleRenderer {
public:
    static const int kWidth = 120;
    static const int kHeight = 40;

    ConsoleRenderer();

    bool Initialize();
    void Clear(char fill = ' ');
    void DrawChar(int x, int y, char ch);
    void DrawText(int x, int y, const std::string& text);
    void DrawSprite(int x, int y, const Sprite& sprite);
    void DrawBox(int x, int y, int width, int height);
    void Present();

private:
    int Index(int x, int y) const;
    void HideCursor();
    void ResizeConsole();

    HANDLE output = INVALID_HANDLE_VALUE;
    std::vector<CHAR_INFO> buffer;
};

#endif  // CONSOLE_RENDERER_H_
