#ifndef TYPES_H_
#define TYPES_H_

#include <string>

struct Vec2 {
  int x = 0;
  int y = 0;
};

enum class FoodType { At, Percent, Caret };

inline char FoodTypeToChar(FoodType type) {
  switch (type) {
    case FoodType::At:
      return '@';
    case FoodType::Percent:
      return '%';
    case FoodType::Caret:
      return '^';
    default:
      return '?';
  }
}

inline bool CharToFoodType(char ch, FoodType* type) {
  if (type == nullptr) {
    return false;
  }

  switch (ch) {
    case '@':
      *type = FoodType::At;
      return true;
    case '%':
      *type = FoodType::Percent;
      return true;
    case '^':
      *type = FoodType::Caret;
      return true;
    default:
      return false;
  }
}

inline std::string FoodTypeToName(FoodType type) {
  switch (type) {
    case FoodType::At:
      return "@ food";
    case FoodType::Percent:
      return "% food";
    case FoodType::Caret:
      return "^ food";
    default:
      return "unknown food";
  }
}

#endif
