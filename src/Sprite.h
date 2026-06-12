#ifndef SPRITE_H_
#define SPRITE_H_

#include <string>
#include <vector>

struct Sprite {
    std::vector<std::string> lines;

    int Width() const;
    int Height() const;
};

#endif  // SPRITE_H_
