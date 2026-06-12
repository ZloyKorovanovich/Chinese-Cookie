#include "AssetManager.h"

#include <fstream>

namespace {

std::string RemoveCarriageReturn(std::string line) {
  if (!line.empty() && line.back() == '\r') {
    line.pop_back();
  }
  return line;
}

bool IsBlockHeader(const std::string& line) {
  return line.size() >= 2 && line.front() == '%' && line.back() == '%';
}

std::string HeaderToName(const std::string& line) {
  return line.substr(1, line.size() - 2);
}

}  // namespace

bool AssetManager::LoadFromFile(const std::string& fileName) {
  std::ifstream input(fileName);
  if (!input.is_open()) {
    return false;
  }

  sprites_.clear();

  std::string currentName;
  Sprite currentSprite;
  bool hasCurrentBlock = false;

  std::string line;
  while (std::getline(input, line)) {
    line = RemoveCarriageReturn(line);

    if (line == "%%") {
      if (hasCurrentBlock) {
        sprites_[currentName] = currentSprite;
      }
      return true;
    }

    if (IsBlockHeader(line)) {
      if (hasCurrentBlock) {
        sprites_[currentName] = currentSprite;
      }

      currentName = HeaderToName(line);
      currentSprite = Sprite{};
      hasCurrentBlock = true;
      continue;
    }

    if (hasCurrentBlock) {
      currentSprite.lines.push_back(line);
    }
  }

  if (hasCurrentBlock) {
    sprites_[currentName] = currentSprite;
  }

  return !sprites_.empty();
}

bool AssetManager::HasSprite(const std::string& name) const {
  return sprites_.find(name) != sprites_.end();
}

const Sprite& AssetManager::GetSprite(const std::string& name) const {
  const auto it = sprites_.find(name);
  if (it == sprites_.end()) {
    return missingSprite_;
  }
  return it->second;
}

void AssetManager::AddSprite(const std::string& name, const Sprite& sprite) {
  sprites_[name] = sprite;
}

std::vector<std::string> AssetManager::MakeFrameList(
    const std::string& prefix,
    const std::vector<std::string>& fallbackNames) const {
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
  for (const std::string& name : fallbackNames) {
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
