#include "Level.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

enum class Section {
    None,
    Settings,
    Map,
    Monsters
};

static const char kSettingsStartTag[] = "[Settings]";
static const char kSettingsEndTag[] = "[/Settings]";
static const char kMapStartTag[] = "[Map]";
static const char kMapEndTag[] = "[/Map]";
static const char kMonstersStartTag[] = "[Monsters]";
static const char kMonstersEndTag[] = "[/Monsters]";

static const char kPlayerHpKey[] = "player_hp";
static const char kPlayerMoveDelayKey[] = "player_move_delay";
static const char kHungerUpdateIntervalKey[] = "hunger_update_interval";
static const char kHungerPerUpdateKey[] = "hunger_per_update";
static const char kCorrectFoodHungerReductionKey[] = "correct_food_hunger_reduction";
static const char kWrongFoodHungerIncreaseKey[] = "wrong_food_hunger_increase";
static const char kMinAggressiveMoveDelayKey[] = "min_aggressive_move_delay";
static const char kFlyingFoodSpeedKey[] = "flying_food_speed";

static constexpr char kCommentMarker = ';';
static constexpr char kMonsterFieldDelimiter = '|';
static constexpr char kSettingDelimiter = '=';

static std::string RemoveCarriageReturn(std::string line) {
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    return line;
}

static std::string Trim(const std::string& value) {
    const std::string whitespace = " \t\r\n";
    const std::size_t first = value.find_first_not_of(whitespace);
    if (first == std::string::npos) {
        return "";
    }

    const std::size_t last = value.find_last_not_of(whitespace);
    return value.substr(first, last - first + 1);
}

static bool IsCommentOrEmpty(const std::string& line) {
    const std::string trimmed = Trim(line);
    return trimmed.empty() || trimmed[0] == kCommentMarker;
}

static std::vector<std::string> Split(const std::string& value, char delimiter) {
    std::vector<std::string> parts;
    std::stringstream stream(value);
    std::string part;

    while (std::getline(stream, part, delimiter)) {
        parts.push_back(Trim(part));
    }

    return parts;
}

static int ToIntOrDefault(const std::string& value, int default_value) {
    if (value.empty()) {
        return default_value;
    }

    char* end = nullptr;
    const long parsed = std::strtol(value.c_str(), &end, 10);
    if (end == value.c_str()) {
        return default_value;
    }

    return static_cast<int>(parsed);
}

static float ToFloatOrDefault(const std::string& value, float default_value) {
    if (value.empty()) {
        return default_value;
    }

    char* end = nullptr;
    const float parsed = std::strtof(value.c_str(), &end);
    if (end == value.c_str()) {
        return default_value;
    }

    return parsed;
}

static bool ParseNeeds(const std::string& value, std::vector<FoodType>* needs) {
    needs->clear();

    for (char ch : value) {
        FoodType type = FoodType::At;
        if (CharToFoodType(ch, &type)) {
            needs->push_back(type);
        }
    }

    return !needs->empty();
}

static bool ParseMonsterLine(const std::string& line, MonsterLevelSpec* monster) {
    const std::vector<std::string> parts = Split(line, kMonsterFieldDelimiter);
    if (parts.size() < 4 || parts[0].empty() || parts[1].empty() || parts[2].empty()) {
        return false;
    }

    monster->name = parts[0];
    monster->map_symbol = parts[1][0];
    monster->animation_name = parts[2];

    if (!ParseNeeds(parts[3], &monster->needs)) {
        return false;
    }

    if (parts.size() >= 5) {
        monster->hunger_limit = ToIntOrDefault(parts[4], monster->hunger_limit);
    }
    if (parts.size() >= 6) {
        monster->damage = ToIntOrDefault(parts[5], monster->damage);
    }
    if (parts.size() >= 7) {
        monster->move_delay = ToIntOrDefault(parts[6], monster->move_delay);
    }
    if (parts.size() >= 8) {
        monster->frame_delay = ToIntOrDefault(parts[7], monster->frame_delay);
    }

    return true;
}

static bool FindMonsterSpecBySymbol(std::vector<MonsterLevelSpec>* monsters, char symbol, int* index) {
    for (int i = 0; i < static_cast<int>(monsters->size()); ++i) {
        if ((*monsters)[i].map_symbol == symbol) {
            *index = i;
            return true;
        }
    }

    return false;
}

static void NormalizeMapWidth(std::vector<std::string>* map) {
    std::size_t width = 0;
    for (const std::string& row : *map) {
        width = std::max(width, row.size());
    }

    for (std::string& row : *map) {
        if (row.size() < width) {
            row += std::string(width - row.size(), kFloorCell);
        }
    }
}

static void ApplySettingLine(const std::string& line, GameSettings* settings) {
    const std::vector<std::string> parts = Split(line, kSettingDelimiter);
    if (parts.size() != 2 || parts[0].empty()) {
        return;
    }

    const std::string& key = parts[0];
    const std::string& value = parts[1];

    if (key == kPlayerHpKey) {
        settings->player_hp = ToIntOrDefault(value, settings->player_hp);
    } else if (key == kPlayerMoveDelayKey) {
        settings->player_move_delay = ToIntOrDefault(value, settings->player_move_delay);
    } else if (key == kHungerUpdateIntervalKey) {
        settings->hunger_update_interval = ToIntOrDefault(value, settings->hunger_update_interval);
    } else if (key == kHungerPerUpdateKey) {
        settings->hunger_per_update = ToIntOrDefault(value, settings->hunger_per_update);
    } else if (key == kCorrectFoodHungerReductionKey) {
        settings->correct_food_hunger_reduction = ToIntOrDefault(value, settings->correct_food_hunger_reduction);
    } else if (key == kWrongFoodHungerIncreaseKey) {
        settings->wrong_food_hunger_increase = ToIntOrDefault(value, settings->wrong_food_hunger_increase);
    } else if (key == kMinAggressiveMoveDelayKey) {
        settings->min_aggressive_move_delay = ToIntOrDefault(value, settings->min_aggressive_move_delay);
    } else if (key == kFlyingFoodSpeedKey) {
        settings->flying_food_speed = ToFloatOrDefault(value, settings->flying_food_speed);
    }
}

static bool ExtractObjectsFromMap(LevelData* level_data) {
    if (level_data->map.empty()) {
        return false;
    }

    bool found_player = false;
    std::vector<bool> found_monsters(level_data->monsters.size(), false);

    for (int y = 0; y < static_cast<int>(level_data->map.size()); ++y) {
        for (int x = 0; x < static_cast<int>(level_data->map[y].size()); ++x) {
            char& cell = level_data->map[y][x];

            FoodType food_type = FoodType::At;
            if (CharToFoodType(cell, &food_type)) {
                level_data->foods.push_back(Food{food_type, Vec2{x, y}, true});
                cell = kFloorCell;
                continue;
            }

            if (cell == kPlayerSymbol) {
                level_data->player_start = Vec2{x, y};
                found_player = true;
                cell = kFloorCell;
                continue;
            }

            int monster_index = -1;
            if (FindMonsterSpecBySymbol(&level_data->monsters, cell, &monster_index)) {
                level_data->monsters[monster_index].position = Vec2{x, y};
                found_monsters[monster_index] = true;
                cell = kFloorCell;
                continue;
            }
        }
    }

    if (!found_player) {
        return false;
    }

    for (bool found_monster : found_monsters) {
        if (!found_monster) {
            return false;
        }
    }

    return !level_data->monsters.empty();
}

bool LevelLoader::LoadFromFile(const std::string& file_name, LevelData* level_data) const {
    if (level_data == nullptr) {
        return false;
    }

    std::ifstream input(file_name);
    if (!input.is_open()) {
        return false;
    }

    LevelData result;
    Section section = Section::None;

    std::string line;
    while (std::getline(input, line)) {
        line = RemoveCarriageReturn(line);
        const std::string trimmed = Trim(line);

        if (trimmed == kSettingsStartTag) {
            section = Section::Settings;
            continue;
        }
        if (trimmed == kSettingsEndTag) {
            section = Section::None;
            continue;
        }
        if (trimmed == kMapStartTag) {
            section = Section::Map;
            continue;
        }
        if (trimmed == kMapEndTag) {
            section = Section::None;
            continue;
        }
        if (trimmed == kMonstersStartTag) {
            section = Section::Monsters;
            continue;
        }
        if (trimmed == kMonstersEndTag) {
            section = Section::None;
            continue;
        }

        if (section == Section::Settings) {
            if (!IsCommentOrEmpty(line)) {
                ApplySettingLine(line, &result.settings);
            }
            continue;
        }

        if (section == Section::Map) {
            if (!line.empty()) {
                result.map.push_back(line);
            }
            continue;
        }

        if (section == Section::Monsters) {
            if (IsCommentOrEmpty(line)) {
                continue;
            }

            MonsterLevelSpec monster;
            if (ParseMonsterLine(line, &monster)) {
                result.monsters.push_back(monster);
            }
            continue;
        }
    }

    NormalizeMapWidth(&result.map);
    if (!ExtractObjectsFromMap(&result)) {
        return false;
    }

    *level_data = result;
    return true;
}
