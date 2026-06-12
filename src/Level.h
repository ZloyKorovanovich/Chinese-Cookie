#ifndef LEVEL_H_
#define LEVEL_H_

#include <string>
#include <vector>

#include "Food.h"
#include "Types.h"

struct MonsterLevelSpec {
  std::string name;
  char mapSymbol = 'M';
  std::string animationPrefix;
  std::vector<FoodType> needs;
  Vec2 position;
  int hungerLimit = 100;
  int damage = 4;
  int moveDelay = 8;
};

struct LevelData {
  std::vector<std::string> map;
  Vec2 playerStart;
  std::vector<Food> foods;
  std::vector<MonsterLevelSpec> monsters;
};

class LevelLoader {
 public:
  bool LoadFromFile(const std::string& fileName, LevelData* levelData) const;
};

#endif
