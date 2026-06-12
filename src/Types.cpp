#include "Types.h"

char FoodTypeToChar(FoodType type) {
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

bool CharToFoodType(char ch, FoodType* type) {
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

std::string FoodTypeToName(FoodType type) {
    const char symbol = FoodTypeToChar(type);
    if (symbol == kMissingFoodSymbol) {
        return "unknown food";
    }
    return std::string(1, symbol) + " food";
}
