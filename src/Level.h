#ifndef LEVEL_H_
#define LEVEL_H_

#include <string>
#include <vector>

#include "Food.h"
#include "Types.h"

struct MonsterLevelSpec {
    std::string name;
    char map_symbol = 'M';
    std::string animation_prefix;
    std::vector<FoodType> needs;
    Vec2 position;
    int hunger_limit = 100;
    int damage = 4;
    int move_delay = 8;
};

struct LevelData {
    std::vector<std::string> map;
    Vec2 player_start;
    std::vector<Food> foods;
    std::vector<MonsterLevelSpec> monsters;
};

class LevelLoader {
public:
    bool LoadFromFile(const std::string& file_name, LevelData* level_data) const;
};

#endif  // LEVEL_H_
