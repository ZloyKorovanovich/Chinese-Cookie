#include "Monster.h"

bool Monster::IsFed() const {
    for (const FoodNeed& need : needs) {
        if (need.state != NeedState::Eaten) {
            return false;
        }
    }
    return true;
}

bool Monster::NeedsFood(FoodType type) const {
    for (const FoodNeed& need : needs) {
        if (need.type == type && need.state == NeedState::Missing) {
            return true;
        }
    }
    return false;
}

int Monster::FirstMissingNeedIndex() const {
    for (int i = 0; i < static_cast<int>(needs.size()); ++i) {
        if (needs[i].state == NeedState::Missing) {
            return i;
        }
    }
    return -1;
}

int Monster::ActiveNeedCount() const {
    int count = 0;
    for (const FoodNeed& need : needs) {
        if (need.state != NeedState::Missing) {
            ++count;
        }
    }
    return count;
}

bool Monster::HasAllNeedCellsActive() const {
    return ActiveNeedCount() == static_cast<int>(needs.size());
}

bool Monster::IsFullyHungry() const {
    return hunger >= hunger_limit;
}
