#include "Food.h"

char Food::Symbol() const {
    return FoodTypeToChar(type);
}
