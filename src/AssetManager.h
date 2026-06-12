#ifndef ASSET_MANAGER_H_
#define ASSET_MANAGER_H_

#include <map>
#include <string>
#include <vector>

#include "Sprite.h"

class AssetManager {
 public:
  bool LoadFromFile(const std::string& fileName);

  bool HasSprite(const std::string& name) const;
  const Sprite& GetSprite(const std::string& name) const;
  void AddSprite(const std::string& name, const Sprite& sprite);

  std::vector<std::string> MakeFrameList(
      const std::string& prefix,
      const std::vector<std::string>& fallbackNames) const;

 private:
  std::map<std::string, Sprite> sprites_;
  Sprite missingSprite_ = Sprite{{"+---+", "| ? |", "+---+"}};
};

#endif
