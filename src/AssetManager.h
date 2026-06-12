#ifndef ASSET_MANAGER_H_
#define ASSET_MANAGER_H_

#include <map>
#include <string>
#include <vector>

#include "Sprite.h"

class AssetManager {
public:
    bool LoadFromFile(const std::string& file_name);

    bool HasSprite(const std::string& name) const;
    bool HasAnimation(const std::string& name) const;
    const Sprite& GetSprite(const std::string& name) const;
    std::vector<std::string> GetAnimationFrames(const std::string& name) const;

private:
    void StoreBlock(const std::string& name, const Sprite& block);

    std::map<std::string, Sprite> sprites;
    std::map<std::string, std::vector<std::string>> animations;
    Sprite missing_sprite;
};

#endif  // ASSET_MANAGER_H_
