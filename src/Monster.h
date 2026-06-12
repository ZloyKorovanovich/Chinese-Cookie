#ifndef MONSTER_H_
#define MONSTER_H_

#include <string>
#include <vector>

#include "Types.h"

enum class NeedState { Missing, Flying, Eaten, WrongFlying, Wrong };

struct FoodNeed {
  FoodType type = FoodType::At;
  FoodType deliveredType = FoodType::At;
  NeedState state = NeedState::Missing;
};

struct Monster {
  std::string name;
  char mapSymbol = 'M';
  Vec2 position;

  std::vector<FoodNeed> needs;

  std::vector<std::string> frameNames;
  int currentFrame = 0;
  int animationTimer = 0;
  int frameDelay = 8;

  int hunger = 0;
  int hungerLimit = 100;
  int damage = 4;

  int moveTimer = 0;
  int moveDelay = 8;
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

  bool IsFullyHungry() const { return hunger >= hungerLimit; }
};

#endif
