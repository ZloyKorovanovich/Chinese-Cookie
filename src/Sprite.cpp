#include "Sprite.h"

int Sprite::Width() const {
    int width = 0;
    for (const std::string& line : lines) {
        if (static_cast<int>(line.size()) > width) {
            width = static_cast<int>(line.size());
        }
    }
    return width;
}

int Sprite::Height() const {
    return static_cast<int>(lines.size());
}
