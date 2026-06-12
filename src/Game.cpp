#include "Game.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <thread>
#include <vector>

namespace {

const int kMapX = 2;
const int kMapY = 2;
const int kMapWidth = 58;
const int kMapHeight = 20;
const int kPanelX = 64;
const int kPanelY = 2;

const int kPlayerMoveDelay = 3;

bool IsKeyDown(int virtualKey) {
  return (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
}

}  // namespace

bool Game::Initialize() {
  if (!renderer_.Initialize()) {
    return false;
  }

  LoadAssets();
  Reset();
  return true;
}

void Game::Run() {
  while (running_) {
    ProcessInput();

    if (state_ == GameState::Playing) {
      Update();
    }

    Draw();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}

void Game::Reset() {
  state_ = GameState::Playing;
  running_ = true;
  tick_ = 0;
  playerMoveCooldown_ = 0;
  previousInput_ = InputState{};
  defeatReason_.clear();

  player_.position = Vec2{4, 4};
  player_.hp = 100;

  flyingFoods_.clear();

  if (LoadLevelFromFile("level.txt")) {
    SetMessage("Level loaded from level.txt. Collect @, %, ^ food.");
  } else {
    BuildMap();
    BuildFoods();
    BuildMonsters();
    SetMessage("level.txt was not loaded. Fallback level is running.");
  }
}

void Game::LoadAssets() {
  assets_.LoadFromFile("assets.txt");

  if (!assets_.HasSprite("missing")) {
    assets_.AddSprite("missing", Sprite{{"+---+", "| ? |", "+---+"}});
  }

  if (!assets_.HasSprite("dragon_0")) {
    assets_.AddSprite("dragon_0",
                      Sprite{{"     /^^^^^/", " <==[ o o ]==>", "     / vv /",
                              "    /_/==/_", "      /  /", "   ^==|  |==^"}});
    assets_.AddSprite("dragon_1",
                      Sprite{{"   /^/^^^/^/", " <==[ ^ ^ ]==>", "     / -- /",
                              "    /_/==/_", "   <  |  |  >", "     ^^^^^^"}});
    assets_.AddSprite("dragon_2",
                      Sprite{{"     /^^^^^/", " <==[ O O ]==>", "     / ~~ /",
                              "    /_/==/_", "   ^  |  |  ^", "     /____/"}});
  }

  if (!assets_.HasSprite("octopus_0")) {
    assets_.AddSprite("octopus_0",
                      Sprite{{" .---. ", "/ o o\\", "\\  ^ /", " /|||\\"}});
    assets_.AddSprite("octopus_1",
                      Sprite{{" .---. ", "/ ^ ^\\", "\\  o /", "//|||\\\\"}});
    assets_.AddSprite("octopus_2",
                      Sprite{{" .---. ", "/ O O\\", "\\  - /", "v ||| v"}});
  }
}

void Game::BuildMap() {
  map_.assign(kMapHeight, std::string(kMapWidth, '.'));

  for (int x = 0; x < kMapWidth; ++x) {
    map_[0][x] = '#';
    map_[kMapHeight - 1][x] = '#';
  }

  for (int y = 0; y < kMapHeight; ++y) {
    map_[y][0] = '#';
    map_[y][kMapWidth - 1] = '#';
  }

  auto setWallLine = [this](int x1, int y, int x2) {
    for (int x = x1; x <= x2; ++x) {
      if (IsInsideMap(x, y)) {
        map_[y][x] = '#';
      }
    }
  };

  setWallLine(13, 3, 21);
  setWallLine(41, 3, 49);
  setWallLine(12, 8, 19);
  setWallLine(37, 8, 44);
  setWallLine(17, 14, 25);
}

void Game::BuildFoods() {
  foods_.clear();
  foods_.push_back(Food{FoodType::At, Vec2{8, 5}, true});
  foods_.push_back(Food{FoodType::Percent, Vec2{22, 7}, true});
  foods_.push_back(Food{FoodType::Caret, Vec2{39, 7}, true});
  foods_.push_back(Food{FoodType::Caret, Vec2{6, 12}, true});
  foods_.push_back(Food{FoodType::At, Vec2{30, 12}, true});
  foods_.push_back(Food{FoodType::Percent, Vec2{49, 14}, true});
  foods_.push_back(Food{FoodType::Percent, Vec2{12, 16}, true});
  foods_.push_back(Food{FoodType::Caret, Vec2{35, 16}, true});
}

void Game::BuildMonsters() {
  monsters_.clear();

  Monster dragon;
  dragon.name = "Dragon";
  dragon.mapSymbol = '$';
  dragon.position = Vec2{28, 5};
  dragon.needs = {FoodNeed{FoodType::At}, FoodNeed{FoodType::Percent},
                  FoodNeed{FoodType::Caret}};
  dragon.frameNames =
      assets_.MakeFrameList("dragon", {"wtf_0", "wtf_1", "wtf_2"});
  dragon.frameDelay = 7;
  dragon.hungerLimit = 80;
  dragon.damage = 4;
  dragon.moveDelay = 8;
  monsters_.push_back(dragon);

  Monster octopus;
  octopus.name = "Octopus";
  octopus.mapSymbol = '&';
  octopus.position = Vec2{31, 10};
  octopus.needs = {FoodNeed{FoodType::Percent}, FoodNeed{FoodType::At},
                   FoodNeed{FoodType::Caret}};
  octopus.frameNames =
      assets_.MakeFrameList("octopus", {"tentacle", "tentacle", "tentacle"});
  octopus.frameDelay = 9;
  octopus.hungerLimit = 95;
  octopus.damage = 3;
  octopus.moveDelay = 10;
  monsters_.push_back(octopus);
}

bool Game::LoadLevelFromFile(const std::string& fileName) {
  LevelLoader loader;
  LevelData levelData;

  if (!loader.LoadFromFile(fileName, &levelData)) {
    return false;
  }

  ApplyLevelData(levelData);
  return true;
}

void Game::ApplyLevelData(const LevelData& levelData) {
  map_ = levelData.map;
  foods_ = levelData.foods;
  player_.position = levelData.playerStart;

  monsters_.clear();
  for (const MonsterLevelSpec& spec : levelData.monsters) {
    Monster monster;
    monster.name = spec.name;
    monster.mapSymbol = spec.mapSymbol;
    monster.position = spec.position;
    monster.hungerLimit = spec.hungerLimit;
    monster.damage = spec.damage;
    monster.moveDelay = spec.moveDelay;
    monster.frameDelay = (spec.mapSymbol == '&') ? 9 : 7;

    for (FoodType type : spec.needs) {
      monster.needs.push_back(FoodNeed{type});
    }

    std::vector<std::string> fallbackNames;
    if (spec.mapSymbol == '&') {
      fallbackNames = {"tentacle", "tentacle", "tentacle"};
    } else if (spec.mapSymbol == '$') {
      fallbackNames = {"dragon_0", "dragon_1", "dragon_2"};
    } else {
      fallbackNames = {"missing", "missing", "missing"};
    }

    monster.frameNames =
        assets_.MakeFrameList(spec.animationPrefix, fallbackNames);
    monsters_.push_back(monster);
  }
}

void Game::ProcessInput() {
  const InputState input = ReadInput();

  if (playerMoveCooldown_ > 0) {
    --playerMoveCooldown_;
  }

  if (JustPressed(input.q, previousInput_.q)) {
    running_ = false;
  }

  if (state_ != GameState::Playing) {
    if (JustPressed(input.r, previousInput_.r)) {
      Reset();
    }
    previousInput_ = input;
    return;
  }

  TryHandlePlayerMovement(input);

  if (JustPressed(input.e, previousInput_.e)) {
    SetMessage(
        "Food is sent automatically when you step on it. Hold movement keys to "
        "run.");
  }

  previousInput_ = input;
}

Game::InputState Game::ReadInput() const {
  InputState input;
  input.w = IsKeyDown('W') || IsKeyDown(VK_UP);
  input.a = IsKeyDown('A') || IsKeyDown(VK_LEFT);
  input.s = IsKeyDown('S') || IsKeyDown(VK_DOWN);
  input.d = IsKeyDown('D') || IsKeyDown(VK_RIGHT);
  input.e = IsKeyDown('E');
  input.r = IsKeyDown('R');
  input.q = IsKeyDown('Q') || IsKeyDown(VK_ESCAPE);
  return input;
}

bool Game::JustPressed(bool current, bool previous) const {
  return current && !previous;
}

void Game::TryHandlePlayerMovement(const InputState& input) {
  if (playerMoveCooldown_ > 0) {
    return;
  }

  int dx = 0;
  int dy = 0;

  if (input.w && !input.s) {
    dy = -1;
  } else if (input.s && !input.w) {
    dy = 1;
  }

  if (input.a && !input.d) {
    dx = -1;
  } else if (input.d && !input.a) {
    dx = 1;
  }

  if (dx != 0 && dy != 0) {
    dx = 0;
  }

  if (dx == 0 && dy == 0) {
    return;
  }

  TryMovePlayer(dx, dy);
  playerMoveCooldown_ = kPlayerMoveDelay;
}

void Game::TryMovePlayer(int dx, int dy) {
  const int nextX = player_.position.x + dx;
  const int nextY = player_.position.y + dy;

  if (IsWall(nextX, nextY)) {
    SetMessage("A wall blocks the path.");
    return;
  }

  if (IsMonsterAt(nextX, nextY, -1)) {
    SetMessage("A monster blocks the path.");
    return;
  }

  player_.position.x = nextX;
  player_.position.y = nextY;

  CheckPlayerNearMonsterDefeat();
  if (state_ == GameState::Playing) {
    CheckFoodPickup();
  }
}

bool Game::IsWall(int x, int y) const {
  if (!IsInsideMap(x, y)) {
    return true;
  }
  return map_[y][x] == '#';
}

bool Game::IsBlockedForMonster(int x, int y, int movingMonsterIndex) const {
  if (IsWall(x, y)) {
    return true;
  }

  if (player_.position.x == x && player_.position.y == y) {
    return true;
  }

  return IsMonsterAt(x, y, movingMonsterIndex);
}

bool Game::IsMonsterAt(int x, int y, int ignoredMonsterIndex) const {
  for (int i = 0; i < static_cast<int>(monsters_.size()); ++i) {
    if (i == ignoredMonsterIndex) {
      continue;
    }

    if (monsters_[i].position.x == x && monsters_[i].position.y == y) {
      return true;
    }
  }

  return false;
}

bool Game::IsInsideMap(int x, int y) const {
  return y >= 0 && y < static_cast<int>(map_.size()) && x >= 0 &&
         x < static_cast<int>(map_[y].size());
}

void Game::Update() {
  ++tick_;
  UpdateMonsterAnimations();

  CheckPlayerNearMonsterDefeat();
  if (state_ != GameState::Playing) {
    return;
  }

  UpdateMonsterMovement();
  CheckPlayerNearMonsterDefeat();
  if (state_ != GameState::Playing) {
    return;
  }

  UpdateHungerAndDamage();
  if (state_ != GameState::Playing) {
    return;
  }

  UpdateFlyingFood();
  if (state_ != GameState::Playing) {
    return;
  }

  CheckWinCondition();
}

void Game::UpdateMonsterAnimations() {
  for (Monster& monster : monsters_) {
    if (monster.frameNames.empty()) {
      continue;
    }

    ++monster.animationTimer;
    if (monster.animationTimer >= monster.frameDelay) {
      monster.animationTimer = 0;
      monster.currentFrame = (monster.currentFrame + 1) %
                             static_cast<int>(monster.frameNames.size());
    }
  }
}

void Game::UpdateMonsterMovement() {
  for (int i = 0; i < static_cast<int>(monsters_.size()); ++i) {
    Monster& monster = monsters_[i];
    if (monster.IsFed()) {
      continue;
    }

    ++monster.moveTimer;
    const int delay = monster.aggressive ? std::max(3, monster.moveDelay / 2)
                                         : monster.moveDelay;
    if (monster.moveTimer < delay) {
      continue;
    }
    monster.moveTimer = 0;

    if (IsAdjacent(monster.position, player_.position)) {
      continue;
    }

    const int dx = Sign(player_.position.x - monster.position.x);
    const int dy = Sign(player_.position.y - monster.position.y);
    const int absDx = std::abs(player_.position.x - monster.position.x);
    const int absDy = std::abs(player_.position.y - monster.position.y);

    std::vector<Vec2> candidates;
    if (absDx >= absDy) {
      candidates.push_back(Vec2{dx, 0});
      candidates.push_back(Vec2{0, dy});
    } else {
      candidates.push_back(Vec2{0, dy});
      candidates.push_back(Vec2{dx, 0});
    }

    for (const Vec2& candidate : candidates) {
      if (candidate.x == 0 && candidate.y == 0) {
        continue;
      }

      const int nextX = monster.position.x + candidate.x;
      const int nextY = monster.position.y + candidate.y;
      if (!IsBlockedForMonster(nextX, nextY, i)) {
        monster.position.x = nextX;
        monster.position.y = nextY;
        break;
      }
    }
  }
}

void Game::UpdateHungerAndDamage() {
  if (tick_ % 20 != 0) {
    return;
  }

  for (Monster& monster : monsters_) {
    if (monster.IsFed()) {
      monster.hunger = 0;
      monster.aggressive = false;
      continue;
    }

    monster.hunger += 3;
    if (monster.hunger >= monster.hungerLimit) {
      monster.hunger = monster.hungerLimit;
      monster.aggressive = true;
    }
  }
}

void Game::UpdateFlyingFood() {
  for (FlyingFood& flyingFood : flyingFoods_) {
    if (!flyingFood.active) {
      continue;
    }

    if (flyingFood.targetMonsterIndex >= 0 &&
        flyingFood.targetMonsterIndex < static_cast<int>(monsters_.size())) {
      flyingFood.targetX = static_cast<float>(
          monsters_[flyingFood.targetMonsterIndex].position.x);
      flyingFood.targetY = static_cast<float>(
          monsters_[flyingFood.targetMonsterIndex].position.y);
    }

    const float dx = flyingFood.targetX - flyingFood.x;
    const float dy = flyingFood.targetY - flyingFood.y;
    const float distance = std::sqrt(dx * dx + dy * dy);

    if (distance <= flyingFood.speed || distance < 0.05f) {
      flyingFood.x = flyingFood.targetX;
      flyingFood.y = flyingFood.targetY;
      const FlyingFood deliveredFood = flyingFood;
      flyingFood.active = false;
      CompleteFoodDelivery(deliveredFood);
      continue;
    }

    flyingFood.x += (dx / distance) * flyingFood.speed;
    flyingFood.y += (dy / distance) * flyingFood.speed;
  }

  flyingFoods_.erase(
      std::remove_if(flyingFoods_.begin(), flyingFoods_.end(),
                     [](const FlyingFood& food) { return !food.active; }),
      flyingFoods_.end());
}

void Game::CheckFoodPickup() {
  for (Food& food : foods_) {
    if (!food.active) {
      continue;
    }

    if (food.position.x == player_.position.x &&
        food.position.y == player_.position.y) {
      StartFoodFlight(food);
      return;
    }
  }
}

void Game::CheckWinCondition() {
  if (AllMonstersFed()) {
    state_ = GameState::Victory;
    SetMessage("All monsters are fed. Balance is restored.");
  }
}

void Game::StartFoodFlight(Food& food) {
  food.active = false;

  int targetMonsterIndex =
      FindCorrectTargetMonster(food.type, player_.position);
  bool correctFood = true;
  int targetNeedIndex = -1;

  if (targetMonsterIndex >= 0) {
    targetNeedIndex = ReserveCorrectMonsterNeed(targetMonsterIndex, food.type);
  } else {
    correctFood = false;
    targetMonsterIndex = FindHungriestMonster();
    if (targetMonsterIndex >= 0) {
      targetNeedIndex = ReserveWrongMonsterNeed(targetMonsterIndex, food.type);
    }
  }

  if (targetMonsterIndex < 0) {
    SetMessage("No monster can receive this food now.");
    return;
  }

  FlyingFood flyingFood;
  flyingFood.type = food.type;
  flyingFood.symbol = food.Symbol();
  flyingFood.x = static_cast<float>(player_.position.x);
  flyingFood.y = static_cast<float>(player_.position.y);
  flyingFood.targetX =
      static_cast<float>(monsters_[targetMonsterIndex].position.x);
  flyingFood.targetY =
      static_cast<float>(monsters_[targetMonsterIndex].position.y);
  flyingFood.targetMonsterIndex = targetMonsterIndex;
  flyingFood.targetNeedIndex = targetNeedIndex;
  flyingFood.correctFood = correctFood;
  flyingFood.active = true;

  flyingFoods_.push_back(flyingFood);

  if (correctFood) {
    SetMessage(std::string("Food ") + food.Symbol() + " flies to " +
               monsters_[targetMonsterIndex].name + ".");
  } else {
    SetMessage(std::string("Wrong food ") + food.Symbol() +
               " flies to the hungriest monster: " +
               monsters_[targetMonsterIndex].name + ".");
  }
}

int Game::FindCorrectTargetMonster(FoodType type, Vec2 start) const {
  int bestIndex = -1;
  int bestDistance = std::numeric_limits<int>::max();

  for (int i = 0; i < static_cast<int>(monsters_.size()); ++i) {
    if (!monsters_[i].NeedsFood(type)) {
      continue;
    }

    const int distance = DistanceSquared(start, monsters_[i].position);
    if (distance < bestDistance) {
      bestDistance = distance;
      bestIndex = i;
    }
  }

  return bestIndex;
}

int Game::FindHungriestMonster() const {
  int bestIndex = -1;
  int bestHunger = -1;
  int bestActiveCells = -1;

  for (int i = 0; i < static_cast<int>(monsters_.size()); ++i) {
    const Monster& monster = monsters_[i];
    if (monster.IsFed()) {
      continue;
    }

    const int activeCells = monster.ActiveNeedCount();
    if (monster.hunger > bestHunger ||
        (monster.hunger == bestHunger && activeCells > bestActiveCells)) {
      bestHunger = monster.hunger;
      bestActiveCells = activeCells;
      bestIndex = i;
    }
  }

  return bestIndex;
}

int Game::ReserveCorrectMonsterNeed(int monsterIndex, FoodType type) {
  if (monsterIndex < 0 || monsterIndex >= static_cast<int>(monsters_.size())) {
    return -1;
  }

  Monster& monster = monsters_[monsterIndex];
  for (int i = 0; i < static_cast<int>(monster.needs.size()); ++i) {
    FoodNeed& need = monster.needs[i];
    if (need.type == type && need.state == NeedState::Missing) {
      need.state = NeedState::Flying;
      need.deliveredType = type;
      return i;
    }
  }

  return -1;
}

int Game::ReserveWrongMonsterNeed(int monsterIndex, FoodType type) {
  if (monsterIndex < 0 || monsterIndex >= static_cast<int>(monsters_.size())) {
    return -1;
  }

  Monster& monster = monsters_[monsterIndex];
  const int missingIndex = monster.FirstMissingNeedIndex();
  if (missingIndex >= 0) {
    monster.needs[missingIndex].state = NeedState::WrongFlying;
    monster.needs[missingIndex].deliveredType = type;
  }

  return missingIndex;
}

void Game::CompleteFoodDelivery(const FlyingFood& flyingFood) {
  if (flyingFood.correctFood) {
    CompleteCorrectMonsterNeed(flyingFood.targetMonsterIndex,
                               flyingFood.targetNeedIndex, flyingFood.type);
  } else {
    CompleteWrongMonsterNeed(flyingFood.targetMonsterIndex,
                             flyingFood.targetNeedIndex, flyingFood.type);
  }
}

void Game::CompleteCorrectMonsterNeed(int monsterIndex, int needIndex,
                                      FoodType type) {
  if (monsterIndex < 0 || monsterIndex >= static_cast<int>(monsters_.size())) {
    return;
  }

  Monster& monster = monsters_[monsterIndex];
  if (needIndex >= 0 && needIndex < static_cast<int>(monster.needs.size())) {
    FoodNeed& need = monster.needs[needIndex];
    if (need.type == type && need.state == NeedState::Flying) {
      need.state = NeedState::Eaten;
      need.deliveredType = type;
      monster.hunger = std::max(0, monster.hunger - 35);
      if (monster.IsFed()) {
        monster.aggressive = false;
        monster.hunger = 0;
        SetMessage(monster.name + " is fed and calm.");
      } else {
        SetMessage(monster.name + " ate correct " + FoodTypeToName(type) + ".");
      }
    }
  }
}

void Game::CompleteWrongMonsterNeed(int monsterIndex, int needIndex,
                                    FoodType type) {
  if (monsterIndex < 0 || monsterIndex >= static_cast<int>(monsters_.size())) {
    return;
  }

  Monster& monster = monsters_[monsterIndex];
  const bool wasFullAndStarving =
      monster.HasAllNeedCellsActive() && monster.IsFullyHungry();

  if (wasFullAndStarving) {
    TriggerDefeat(monster.name + " was fully hungry and rejected wrong food.");
    return;
  }

  if (needIndex >= 0 && needIndex < static_cast<int>(monster.needs.size())) {
    FoodNeed& need = monster.needs[needIndex];
    if (need.state == NeedState::WrongFlying) {
      need.state = NeedState::Wrong;
      need.deliveredType = type;
    }
  }

  monster.hunger += 35;
  if (monster.hunger >= monster.hungerLimit) {
    monster.hunger = monster.hungerLimit;
    monster.aggressive = true;
  }

  if (needIndex >= 0) {
    SetMessage(monster.name + " received wrong " + FoodTypeToName(type) +
               ". One hunger cell is activated.");
  } else {
    SetMessage(monster.name + " received wrong " + FoodTypeToName(type) +
               ". All hunger cells are already active.");
  }
}

void Game::TriggerDefeat(const std::string& reason) {
  defeatReason_ = reason;
  player_.hp = 0;
  state_ = GameState::Defeat;
  SetMessage(reason);
}

void Game::CheckPlayerNearMonsterDefeat() {
  if (state_ != GameState::Playing) {
    return;
  }

  for (const Monster& monster : monsters_) {
    if (monster.IsFed()) {
      continue;
    }

    if (IsAdjacent(player_.position, monster.position)) {
      TriggerDefeat(monster.name + " got too close to the player.");
      return;
    }
  }
}

bool Game::AllMonstersFed() const {
  for (const Monster& monster : monsters_) {
    if (!monster.IsFed()) {
      return false;
    }
  }
  return true;
}

void Game::Draw() {
  renderer_.Clear();

  if (state_ == GameState::Victory) {
    DrawEndScreen("VICTORY", "All Ling Shou creatures are fed.");
    renderer_.Present();
    return;
  }

  if (state_ == GameState::Defeat) {
    const std::string subtitle = defeatReason_.empty()
                                     ? "The hungry spirits overcame you."
                                     : defeatReason_;
    DrawEndScreen("DEFEAT", subtitle);
    renderer_.Present();
    return;
  }

  DrawMap();
  DrawMonsterPanels();
  DrawHud();
  renderer_.Present();
}

void Game::DrawMap() {
  const int mapHeight = static_cast<int>(map_.size());
  const int mapWidth =
      mapHeight > 0 ? static_cast<int>(map_[0].size()) : kMapWidth;

  renderer_.DrawBox(kMapX - 1, kMapY - 1, mapWidth + 2, mapHeight + 2);
  renderer_.DrawText(kMapX, kMapY - 1, " World ");

  for (int y = 0; y < static_cast<int>(map_.size()); ++y) {
    for (int x = 0; x < static_cast<int>(map_[y].size()); ++x) {
      renderer_.DrawChar(kMapX + x, kMapY + y, map_[y][x]);
    }
  }

  for (const Food& food : foods_) {
    if (food.active) {
      renderer_.DrawChar(kMapX + food.position.x, kMapY + food.position.y,
                         food.Symbol());
    }
  }

  for (const Monster& monster : monsters_) {
    renderer_.DrawChar(kMapX + monster.position.x, kMapY + monster.position.y,
                       monster.mapSymbol);
  }

  for (const FlyingFood& food : flyingFoods_) {
    if (food.active) {
      const int x = static_cast<int>(std::round(food.x));
      const int y = static_cast<int>(std::round(food.y));
      renderer_.DrawChar(kMapX + x, kMapY + y, food.symbol);
    }
  }

  renderer_.DrawChar(kMapX + player_.position.x, kMapY + player_.position.y,
                     'P');
}

void Game::DrawHud() {
  const int hudY = kMapY + static_cast<int>(map_.size()) + 3;
  renderer_.DrawBox(1, hudY - 1, 118, 8);
  renderer_.DrawText(3, hudY, "HP: " + std::to_string(player_.hp));
  renderer_.DrawText(
      15, hudY,
      "Controls: hold WASD/arrows to move | E info | R restart | Q/Esc quit");
  renderer_.DrawText(
      3, hudY + 2,
      "Legend: P player | @ % ^ food | $ dragon | & octopus | # wall");
  renderer_.DrawText(3, hudY + 4, "Message: " + message_.substr(0, 100));
}

void Game::DrawMonsterPanels() {
  renderer_.DrawBox(kPanelX - 1, kPanelY - 1, 54, 23);
  renderer_.DrawText(kPanelX, kPanelY - 1, " Monsters ");

  for (int i = 0; i < static_cast<int>(monsters_.size()); ++i) {
    const Monster& monster = monsters_[i];
    const int baseY = kPanelY + i * 11;

    renderer_.DrawText(kPanelX, baseY,
                       monster.name + " [" + monster.mapSymbol + "]");
    DrawNeedBar(monster, kPanelX, baseY + 1);

    std::string state = "Chasing";
    if (monster.IsFed()) {
      state = "Fed";
    } else if (monster.aggressive) {
      state = "Aggressive";
    }
    renderer_.DrawText(kPanelX, baseY + 2,
                       "State: " + state +
                           " | Hunger: " + std::to_string(monster.hunger) +
                           "/" + std::to_string(monster.hungerLimit));

    if (!monster.frameNames.empty()) {
      const std::string& frameName = monster.frameNames[monster.currentFrame];
      renderer_.DrawSprite(kPanelX, baseY + 4, assets_.GetSprite(frameName));
    }
  }
}

void Game::DrawNeedBar(const Monster& monster, int x, int y) {
  renderer_.DrawText(x, y, "Needs: ");

  int cursorX = x + 7;
  for (const FoodNeed& need : monster.needs) {
    char ch = FoodTypeToChar(need.type);
    if (need.state == NeedState::Flying ||
        need.state == NeedState::WrongFlying) {
      ch = '*';
    } else if (need.state == NeedState::Eaten) {
      ch = '_';
    } else if (need.state == NeedState::Wrong) {
      ch = FoodTypeToChar(need.deliveredType);
    }

    renderer_.DrawChar(cursorX, y, '[');
    renderer_.DrawChar(cursorX + 1, y, ch);
    renderer_.DrawChar(cursorX + 2, y, ']');
    cursorX += 4;
  }
}

void Game::DrawEndScreen(const std::string& title,
                         const std::string& subtitle) {
  const int boxWidth = 68;
  const int boxHeight = 13;
  const int x = (ConsoleRenderer::kWidth - boxWidth) / 2;
  const int y = (ConsoleRenderer::kHeight - boxHeight) / 2;

  renderer_.DrawBox(x, y, boxWidth, boxHeight);
  renderer_.DrawText(x + (boxWidth - static_cast<int>(title.size())) / 2, y + 2,
                     title);
  renderer_.DrawText(x + (boxWidth - static_cast<int>(subtitle.size())) / 2,
                     y + 5, subtitle);

  if (title == "VICTORY") {
    renderer_.DrawText(x + 13, y + 7,
                       "Balance is restored. The spirits are calm.");
    renderer_.DrawText(x + 18, y + 9, "All monsters are fed.");
  } else {
    renderer_.DrawText(x + 18, y + 8, "Try again and feed them faster.");
  }

  renderer_.DrawText(x + 18, y + boxHeight - 2,
                     "Press R to restart or Q/Esc to quit.");
}

int Game::DistanceSquared(Vec2 a, Vec2 b) const {
  const int dx = a.x - b.x;
  const int dy = a.y - b.y;
  return dx * dx + dy * dy;
}

bool Game::IsAdjacent(Vec2 a, Vec2 b) const {
  const int dx = std::abs(a.x - b.x);
  const int dy = std::abs(a.y - b.y);
  return dx <= 1 && dy <= 1;
}

int Game::Sign(int value) const {
  if (value < 0) {
    return -1;
  }
  if (value > 0) {
    return 1;
  }
  return 0;
}

void Game::SetMessage(const std::string& message) { message_ = message; }
