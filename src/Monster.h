#ifndef MONSTER_H_
#define MONSTER_H_

#include <string>
#include <vector>

#include "Types.h"

enum class NeedState {
    Missing,
    Flying,
    Eaten,
    WrongFlying,
    Wrong
};

struct FoodNeed {
    FoodType type = FoodType::At;
    FoodType delivered_type = FoodType::At;
    NeedState state = NeedState::Missing;
};

struct Monster {
    std::string name;
    char map_symbol = kMissingFoodSymbol;
    Vec2 position;

    std::vector<FoodNeed> needs;

    std::vector<std::string> frame_names;
    int current_frame = 0;
    int animation_timer = 0;
    int frame_delay = kDefaultMonsterFrameDelay;

    int hunger = 0;
    int hunger_limit = kDefaultHungerLimit;
    int damage = kDefaultMonsterDamage;

    int move_timer = 0;
    int move_delay = kDefaultMonsterMoveDelay;
    bool aggressive = false;

    bool IsFed() const;
    bool NeedsFood(FoodType type) const;
    int FirstMissingNeedIndex() const;
    int ActiveNeedCount() const;
    bool HasAllNeedCellsActive() const;
    bool IsFullyHungry() const;
};

#endif  // MONSTER_H_
