#include "AssetManager.h"

#include <fstream>

static std::string RemoveCarriageReturn(std::string line) {
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    return line;
}

static bool IsBlockHeader(const std::string& line) {
    return line.size() >= 2 && line.front() == '%' && line.back() == '%';
}

static std::string HeaderToName(const std::string& line) {
    return line.substr(1, line.size() - 2);
}


bool AssetManager::LoadFromFile(const std::string& file_name) {
    std::ifstream input(file_name);
    if (!input.is_open()) {
        return false;
    }

    sprites.clear();

    std::string current_name;
    Sprite current_sprite;
    bool has_current_block = false;

    std::string line;
    while (std::getline(input, line)) {
        line = RemoveCarriageReturn(line);

        if (line == "%%") {
            if (has_current_block) {
                sprites[current_name] = current_sprite;
            }
            return true;
        }

        if (IsBlockHeader(line)) {
            if (has_current_block) {
                sprites[current_name] = current_sprite;
            }

            current_name = HeaderToName(line);
            current_sprite = Sprite{};
            has_current_block = true;
            continue;
        }

        if (has_current_block) {
            current_sprite.lines.push_back(line);
        }
    }

    if (has_current_block) {
        sprites[current_name] = current_sprite;
    }

    return !sprites.empty();
}

bool AssetManager::HasSprite(const std::string& name) const {
    return sprites.find(name) != sprites.end();
}

const Sprite& AssetManager::GetSprite(const std::string& name) const {
    const auto it = sprites.find(name);
    if (it == sprites.end()) {
        return missing_sprite;
    }
    return it->second;
}

void AssetManager::AddSprite(const std::string& name, const Sprite& sprite) {
    sprites[name] = sprite;
}

std::vector<std::string> AssetManager::MakeFrameList(
    const std::string& prefix,
    const std::vector<std::string>& fallback_names) const {
    std::vector<std::string> result;

    for (int i = 0; i < 3; ++i) {
        const std::string name = prefix + "_" + std::to_string(i);
        if (HasSprite(name)) {
            result.push_back(name);
        }
    }

    if (static_cast<int>(result.size()) == 3) {
        return result;
    }

    result.clear();
    for (const std::string& name : fallback_names) {
        if (HasSprite(name)) {
            result.push_back(name);
        }
    }

    if (result.empty()) {
        result.push_back("missing");
        result.push_back("missing");
        result.push_back("missing");
    }

    while (static_cast<int>(result.size()) < 3) {
        result.push_back(result.back());
    }

    if (static_cast<int>(result.size()) > 3) {
        result.resize(3);
    }

    return result;
}
