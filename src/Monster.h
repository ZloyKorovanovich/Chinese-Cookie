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

    bool IsFed() const {
        for (const FoodNeed& need : needs) {
            if (need.state != NeedState::Eaten) {
                return false;
            }
        }
        return true;
    }

    bool NeedsFood(FoodType type) const {
        for (const FoodNeed& need : needs) {
            if (need.type == type && need.state == NeedState::Missing) {
                return true;
            }
        }
        return false;
    }

    int FirstMissingNeedIndex() const {
        for (int i = 0; i < static_cast<int>(needs.size()); ++i) {
            if (needs[i].state == NeedState::Missing) {
                return i;
            }
        }
        return -1;
    }

    int ActiveNeedCount() const {
        int count = 0;
        for (const FoodNeed& need : needs) {
            if (need.state != NeedState::Missing) {
                ++count;
            }
        }
        return count;
    }

    bool HasAllNeedCellsActive() const {
        return ActiveNeedCount() == static_cast<int>(needs.size());
    }

    bool IsFullyHungry() const {
        return hunger >= hunger_limit;
    }
};

#endif  // MONSTER_H_
