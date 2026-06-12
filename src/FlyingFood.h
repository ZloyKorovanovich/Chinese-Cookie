#ifndef FLYING_FOOD_H_
#define FLYING_FOOD_H_

#include "Types.h"

struct FlyingFood {
    FoodType type = FoodType::At;
    char symbol = kFoodAtSymbol;

    float x = 0.0f;
    float y = 0.0f;
    float target_x = 0.0f;
    float target_y = 0.0f;
    float speed = kDefaultFlyingFoodSpeed;

    int target_monster_index = -1;
    int target_need_index = -1;
    bool correct_food = true;
    bool active = false;
};

#endif  // FLYING_FOOD_H_
