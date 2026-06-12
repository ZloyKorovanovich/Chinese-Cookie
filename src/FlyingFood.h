#ifndef FLYING_FOOD_H_
#define FLYING_FOOD_H_

#include "Types.h"

struct FlyingFood {
  FoodType type = FoodType::At;
  char symbol = '@';

  float x = 0.0f;
  float y = 0.0f;
  float targetX = 0.0f;
  float targetY = 0.0f;
  float speed = 0.65f;

  int targetMonsterIndex = -1;
  int targetNeedIndex = -1;
  bool correctFood = true;
  bool active = false;
};

#endif
