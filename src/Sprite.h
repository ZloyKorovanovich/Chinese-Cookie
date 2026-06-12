#ifndef SPRITE_H_
#define SPRITE_H_

#include <string>
#include <vector>

struct Sprite {
    std::vector<std::string> lines;

    int Width() const {
        int width = 0;
        for (const std::string& line : lines) {
            if (static_cast<int>(line.size()) > width) {
                width = static_cast<int>(line.size());
            }
        }
        return width;
    }

    int Height() const {
        return static_cast<int>(lines.size());
    }
};

#endif  // SPRITE_H_
