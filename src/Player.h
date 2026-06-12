#ifndef PLAYER_H_
#define PLAYER_H_

#include "Types.h"

struct Player {
    Vec2 position;
    int hp = kDefaultPlayerHp;
};

#endif  // PLAYER_H_
