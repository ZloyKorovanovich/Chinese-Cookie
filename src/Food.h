#ifndef FOOD_H_
#define FOOD_H_

#include "Types.h"

struct Food {
    FoodType type = FoodType::At;
    Vec2 position;
    bool active = true;

    char Symbol() const;
};

#endif  // FOOD_H_
