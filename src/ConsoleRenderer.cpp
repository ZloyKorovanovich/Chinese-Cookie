#include "ConsoleRenderer.h"

ConsoleRenderer::ConsoleRenderer() : buffer_(kWidth * kHeight) {}

bool ConsoleRenderer::Initialize() {
  output_ = GetStdHandle(STD_OUTPUT_HANDLE);
  if (output_ == INVALID_HANDLE_VALUE || output_ == nullptr) {
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
  for (CHAR_INFO& cell : buffer_) {
    cell.Char.AsciiChar = fill;
    cell.Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
  }
}

void ConsoleRenderer::DrawChar(int x, int y, char ch) {
  if (x < 0 || x >= kWidth || y < 0 || y >= kHeight) {
    return;
  }

  buffer_[Index(x, y)].Char.AsciiChar = ch;
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
  COORD bufferSize;
  bufferSize.X = static_cast<SHORT>(kWidth);
  bufferSize.Y = static_cast<SHORT>(kHeight);

  COORD bufferCoord;
  bufferCoord.X = 0;
  bufferCoord.Y = 0;

  SMALL_RECT writeRegion;
  writeRegion.Left = 0;
  writeRegion.Top = 0;
  writeRegion.Right = static_cast<SHORT>(kWidth - 1);
  writeRegion.Bottom = static_cast<SHORT>(kHeight - 1);

  WriteConsoleOutputA(output_, buffer_.data(), bufferSize, bufferCoord,
                      &writeRegion);
}

int ConsoleRenderer::Index(int x, int y) const { return y * kWidth + x; }

void ConsoleRenderer::HideCursor() {
  CONSOLE_CURSOR_INFO cursorInfo;
  cursorInfo.dwSize = 1;
  cursorInfo.bVisible = FALSE;
  SetConsoleCursorInfo(output_, &cursorInfo);
}

void ConsoleRenderer::ResizeConsole() {
  SMALL_RECT tinyRect;
  tinyRect.Left = 0;
  tinyRect.Top = 0;
  tinyRect.Right = 1;
  tinyRect.Bottom = 1;
  SetConsoleWindowInfo(output_, TRUE, &tinyRect);

  COORD size;
  size.X = static_cast<SHORT>(kWidth);
  size.Y = static_cast<SHORT>(kHeight);
  SetConsoleScreenBufferSize(output_, size);

  SMALL_RECT rect;
  rect.Left = 0;
  rect.Top = 0;
  rect.Right = static_cast<SHORT>(kWidth - 1);
  rect.Bottom = static_cast<SHORT>(kHeight - 1);
  SetConsoleWindowInfo(output_, TRUE, &rect);
}
