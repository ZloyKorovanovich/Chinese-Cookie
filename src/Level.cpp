#include "Level.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

enum class Section {
    None,
    Map,
    Monsters
};

std::string RemoveCarriageReturn(std::string line) {
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    return line;
}

std::string Trim(const std::string& value) {
    const std::string whitespace = " \t\r\n";
    const std::size_t first = value.find_first_not_of(whitespace);
    if (first == std::string::npos) {
        return "";
    }

    const std::size_t last = value.find_last_not_of(whitespace);
    return value.substr(first, last - first + 1);
}

bool IsCommentOrEmpty(const std::string& line) {
    const std::string trimmed = Trim(line);
    return trimmed.empty() || trimmed[0] == ';' || trimmed[0] == '#';
}

std::vector<std::string> Split(const std::string& value, char delimiter) {
    std::vector<std::string> parts;
    std::stringstream stream(value);
    std::string part;

    while (std::getline(stream, part, delimiter)) {
        parts.push_back(Trim(part));
    }

    return parts;
}

int ToIntOrDefault(const std::string& value, int default_value) {
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

bool ParseNeeds(const std::string& value, std::vector<FoodType>* needs) {
    needs->clear();

    for (char ch : value) {
        FoodType type = FoodType::At;
        if (CharToFoodType(ch, &type)) {
            needs->push_back(type);
        }
    }

    return !needs->empty();
}

bool ParseMonsterLine(const std::string& line, MonsterLevelSpec* monster) {
    const std::vector<std::string> parts = Split(line, '|');
    if (parts.size() < 4 || parts[0].empty() || parts[1].empty() || parts[2].empty()) {
        return false;
    }

    monster->name = parts[0];
    monster->map_symbol = parts[1][0];
    monster->animation_prefix = parts[2];

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

    return true;
}

bool FindMonsterSpecBySymbol(std::vector<MonsterLevelSpec>* monsters, char symbol, int* index) {
    for (int i = 0; i < static_cast<int>(monsters->size()); ++i) {
        if ((*monsters)[i].map_symbol == symbol) {
            *index = i;
            return true;
        }
    }

    return false;
}

void NormalizeMapWidth(std::vector<std::string>* map) {
    std::size_t width = 0;
    for (const std::string& row : *map) {
        width = std::max(width, row.size());
    }

    for (std::string& row : *map) {
        if (row.size() < width) {
            row += std::string(width - row.size(), '.');
        }
    }
}

bool ExtractObjectsFromMap(LevelData* level_data) {
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
                cell = '.';
                continue;
            }

            if (cell == 'P') {
                level_data->player_start = Vec2{x, y};
                found_player = true;
                cell = '.';
                continue;
            }

            int monster_index = -1;
            if (FindMonsterSpecBySymbol(&level_data->monsters, cell, &monster_index)) {
                level_data->monsters[monster_index].position = Vec2{x, y};
                found_monsters[monster_index] = true;
                cell = '.';
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

}  // namespace

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

        if (trimmed == "[Map]") {
            section = Section::Map;
            continue;
        }
        if (trimmed == "[/Map]") {
            section = Section::None;
            continue;
        }
        if (trimmed == "[Monsters]") {
            section = Section::Monsters;
            continue;
        }
        if (trimmed == "[/Monsters]") {
            section = Section::None;
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
