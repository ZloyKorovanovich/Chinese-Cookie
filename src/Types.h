#ifndef TYPES_H_
#define TYPES_H_

#include <string>

#include "Config.h"

struct Vec2 {
    int x = 0;
    int y = 0;
};

enum class FoodType {
    At,
    Percent,
    Caret
};

char FoodTypeToChar(FoodType type);
bool CharToFoodType(char ch, FoodType* type);
std::string FoodTypeToName(FoodType type);

#endif  // TYPES_H_
