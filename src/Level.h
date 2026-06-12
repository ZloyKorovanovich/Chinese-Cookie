#ifndef LEVEL_H_
#define LEVEL_H_

#include <string>
#include <vector>

#include "Config.h"
#include "Food.h"
#include "Types.h"

struct GameSettings {
    int player_hp = kDefaultPlayerHp;
    int player_move_delay = kDefaultPlayerMoveDelay;
    int hunger_update_interval = kDefaultHungerUpdateInterval;
    int hunger_per_update = kDefaultHungerPerUpdate;
    int correct_food_hunger_reduction = kDefaultCorrectFoodHungerReduction;
    int wrong_food_hunger_increase = kDefaultWrongFoodHungerIncrease;
    int min_aggressive_move_delay = kDefaultMinAggressiveMoveDelay;
    float flying_food_speed = kDefaultFlyingFoodSpeed;
};

struct MonsterLevelSpec {
    std::string name;
    char map_symbol = kMissingFoodSymbol;
    std::string animation_name;
    std::vector<FoodType> needs;
    Vec2 position;
    int hunger_limit = kDefaultHungerLimit;
    int damage = kDefaultMonsterDamage;
    int move_delay = kDefaultMonsterMoveDelay;
    int frame_delay = kDefaultMonsterFrameDelay;
};

struct LevelData {
    GameSettings settings;
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
