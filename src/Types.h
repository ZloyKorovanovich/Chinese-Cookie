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

inline char FoodTypeToChar(FoodType type) {
    switch (type) {
        case FoodType::At:
            return kFoodAtSymbol;
        case FoodType::Percent:
            return kFoodPercentSymbol;
        case FoodType::Caret:
            return kFoodCaretSymbol;
        default:
            return kMissingFoodSymbol;
    }
}


inline bool CharToFoodType(char ch, FoodType* type) {
    if (type == nullptr) {
        return false;
    }

    switch (ch) {
        case kFoodAtSymbol:
            *type = FoodType::At;
            return true;
        case kFoodPercentSymbol:
            *type = FoodType::Percent;
            return true;
        case kFoodCaretSymbol:
            *type = FoodType::Caret;
            return true;
        default:
            return false;
    }
}

inline std::string FoodTypeToName(FoodType type) {
    const char symbol = FoodTypeToChar(type);
    if (symbol == kMissingFoodSymbol) {
        return "unknown food";
    }
    return std::string(1, symbol) + " food";
}

#endif  // TYPES_H_
