#include "AssetManager.h"

#include <fstream>

#include "Config.h"

static std::string RemoveCarriageReturn(std::string line) {
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    return line;
}

static bool IsBlockHeader(const std::string& line) {
    return line.size() >= kMinimumAssetHeaderLength &&
           line.front() == kAssetBlockMarker &&
           line.back() == kAssetBlockMarker;
}

static std::string HeaderToName(const std::string& line) {
    return line.substr(kHeaderContentOffset, line.size() - kHeaderMarkerCount);
}

static bool StartsWith(const std::string& text, const std::string& prefix) {
    return text.size() >= prefix.size() && text.compare(0, prefix.size(), prefix) == 0;
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

static bool IsAnimationBlockName(const std::string& name) {
    return StartsWith(name, kAnimationBlockPrefix);
}

static std::string AnimationNameFromBlockName(const std::string& name) {
    return name.substr(std::string(kAnimationBlockPrefix).size());
}

AssetManager::AssetManager() {
    missing_sprite.lines = {
        "[missing]",
        "sprite"
    };
}

bool AssetManager::LoadFromFile(const std::string& file_name) {
    std::ifstream input(file_name);
    if (!input.is_open()) {
        return false;
    }

    sprites.clear();
    animations.clear();

    std::string current_name;
    Sprite current_block;
    bool has_current_block = false;

    std::string line;
    while (std::getline(input, line)) {
        line = RemoveCarriageReturn(line);

        if (line == kAssetEndMarker) {
            if (has_current_block) {
                StoreBlock(current_name, current_block);
            }
            return true;
        }

        if (IsBlockHeader(line)) {
            if (has_current_block) {
                StoreBlock(current_name, current_block);
            }

            current_name = HeaderToName(line);
            current_block = Sprite{};
            has_current_block = true;
            continue;
        }

        if (has_current_block) {
            current_block.lines.push_back(line);
        }
    }

    if (has_current_block) {
        StoreBlock(current_name, current_block);
    }

    return !sprites.empty() || !animations.empty();
}

bool AssetManager::HasSprite(const std::string& name) const {
    return sprites.find(name) != sprites.end();
}

bool AssetManager::HasAnimation(const std::string& name) const {
    return animations.find(name) != animations.end();
}

const Sprite& AssetManager::GetSprite(const std::string& name) const {
    const auto it = sprites.find(name);
    if (it == sprites.end()) {
        return missing_sprite;
    }
    return it->second;
}

std::vector<std::string> AssetManager::GetAnimationFrames(const std::string& name) const {
    const auto it = animations.find(name);
    if (it == animations.end()) {
        return {};
    }
    return it->second;
}

void AssetManager::StoreBlock(const std::string& name, const Sprite& block) {
    if (IsAnimationBlockName(name)) {
        std::vector<std::string> frame_names;
        for (const std::string& line : block.lines) {
            const std::string frame_name = Trim(line);
            if (!frame_name.empty() && frame_name[0] != kAssetCommentMarker) {
                frame_names.push_back(frame_name);
            }
        }
        animations[AnimationNameFromBlockName(name)] = frame_names;
        return;
    }

    sprites[name] = block;
}
