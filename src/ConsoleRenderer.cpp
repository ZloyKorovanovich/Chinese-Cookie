#include "ConsoleRenderer.h"

ConsoleRenderer::ConsoleRenderer()
    : buffer(kWidth * kHeight) {}

bool ConsoleRenderer::Initialize() {
    output = GetStdHandle(STD_OUTPUT_HANDLE);
    if (output == INVALID_HANDLE_VALUE || output == nullptr) {
        return false;
    }

    SetConsoleOutputCP(437);
    ResizeConsole();
    HideCursor();
    Clear();
    Present();
    return true;
}

void ConsoleRenderer::Clear(char fill) {
    for (CHAR_INFO& cell : buffer) {
        cell.Char.AsciiChar = fill;
        cell.Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    }
}

void ConsoleRenderer::DrawChar(int x, int y, char ch) {
    if (x < 0 || x >= kWidth || y < 0 || y >= kHeight) {
        return;
    }

    buffer[Index(x, y)].Char.AsciiChar = ch;
}

void ConsoleRenderer::DrawText(int x, int y, const std::string& text) {
    for (int i = 0; i < static_cast<int>(text.size()); ++i) {
        DrawChar(x + i, y, text[i]);
    }
}

void ConsoleRenderer::DrawSprite(int x, int y, const Sprite& sprite) {
    for (int row = 0; row < static_cast<int>(sprite.lines.size()); ++row) {
        DrawText(x, y + row, sprite.lines[row]);
    }
}

void ConsoleRenderer::DrawBox(int x, int y, int width, int height) {
    if (width < 2 || height < 2) {
        return;
    }

    DrawChar(x, y, '+');
    DrawChar(x + width - 1, y, '+');
    DrawChar(x, y + height - 1, '+');
    DrawChar(x + width - 1, y + height - 1, '+');

    for (int i = 1; i < width - 1; ++i) {
        DrawChar(x + i, y, '-');
        DrawChar(x + i, y + height - 1, '-');
    }

    for (int j = 1; j < height - 1; ++j) {
        DrawChar(x, y + j, '|');
        DrawChar(x + width - 1, y + j, '|');
    }
}

void ConsoleRenderer::Present() {
    COORD buffer_size;
    buffer_size.X = static_cast<SHORT>(kWidth);
    buffer_size.Y = static_cast<SHORT>(kHeight);

    COORD buffer_coord;
    buffer_coord.X = 0;
    buffer_coord.Y = 0;

    SMALL_RECT write_region;
    write_region.Left = 0;
    write_region.Top = 0;
    write_region.Right = static_cast<SHORT>(kWidth - 1);
    write_region.Bottom = static_cast<SHORT>(kHeight - 1);

    WriteConsoleOutputA(output, buffer.data(), buffer_size, buffer_coord, &write_region);
}

int ConsoleRenderer::Index(int x, int y) const {
    return y * kWidth + x;
}

void ConsoleRenderer::HideCursor() {
    CONSOLE_CURSOR_INFO cursor_info;
    cursor_info.dwSize = 1;
    cursor_info.bVisible = FALSE;
    SetConsoleCursorInfo(output, &cursor_info);
}

void ConsoleRenderer::ResizeConsole() {
    SMALL_RECT tiny_rect;
    tiny_rect.Left = 0;
    tiny_rect.Top = 0;
    tiny_rect.Right = 1;
    tiny_rect.Bottom = 1;
    SetConsoleWindowInfo(output, TRUE, &tiny_rect);

    COORD size;
    size.X = static_cast<SHORT>(kWidth);
    size.Y = static_cast<SHORT>(kHeight);
    SetConsoleScreenBufferSize(output, size);

    SMALL_RECT rect;
    rect.Left = 0;
    rect.Top = 0;
    rect.Right = static_cast<SHORT>(kWidth - 1);
    rect.Bottom = static_cast<SHORT>(kHeight - 1);
    SetConsoleWindowInfo(output, TRUE, &rect);
}
