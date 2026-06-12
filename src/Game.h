#ifndef GAME_H_
#define GAME_H_

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <string>
#include <vector>

#include "AssetManager.h"
#include "ConsoleRenderer.h"
#include "FlyingFood.h"
#include "Food.h"
#include "Level.h"
#include "Monster.h"
#include "Player.h"

class Game {
 public:
  bool Initialize();
  void Run();

 private:
  enum class GameState { Playing, Victory, Defeat };

  struct InputState {
    bool w = false;
    bool a = false;
    bool s = false;
    bool d = false;
    bool e = false;
    bool r = false;
    bool q = false;
  };

  void Reset();
  void LoadAssets();
  void BuildMap();
  void BuildFoods();
  void BuildMonsters();
  bool LoadLevelFromFile(const std::string& fileName);
  void ApplyLevelData(const LevelData& levelData);

  void ProcessInput();
  InputState ReadInput() const;
  bool JustPressed(bool current, bool previous) const;

  void TryHandlePlayerMovement(const InputState& input);
  void TryMovePlayer(int dx, int dy);
  bool IsWall(int x, int y) const;
  bool IsBlockedForMonster(int x, int y, int movingMonsterIndex) const;
  bool IsMonsterAt(int x, int y, int ignoredMonsterIndex) const;
  bool IsInsideMap(int x, int y) const;

  void Update();
  void UpdateMonsterAnimations();
  void UpdateHungerAndDamage();
  void UpdateMonsterMovement();
  void UpdateFlyingFood();
  void CheckFoodPickup();
  void CheckWinCondition();
  void CheckPlayerNearMonsterDefeat();

  void StartFoodFlight(Food& food);
  int FindCorrectTargetMonster(FoodType type, Vec2 start) const;
  int FindHungriestMonster() const;
  int ReserveCorrectMonsterNeed(int monsterIndex, FoodType type);
  int ReserveWrongMonsterNeed(int monsterIndex, FoodType type);
  void CompleteFoodDelivery(const FlyingFood& flyingFood);
  void CompleteCorrectMonsterNeed(int monsterIndex, int needIndex,
                                  FoodType type);
  void CompleteWrongMonsterNeed(int monsterIndex, int needIndex, FoodType type);
  void TriggerDefeat(const std::string& reason);
  bool AllMonstersFed() const;

  void Draw();
  void DrawMap();
  void DrawHud();
  void DrawMonsterPanels();
  void DrawNeedBar(const Monster& monster, int x, int y);
  void DrawEndScreen(const std::string& title, const std::string& subtitle);

  int DistanceSquared(Vec2 a, Vec2 b) const;
  bool IsAdjacent(Vec2 a, Vec2 b) const;
  int Sign(int value) const;
  void SetMessage(const std::string& message);

  ConsoleRenderer renderer_;
  AssetManager assets_;

  Player player_;
  std::vector<std::string> map_;
  std::vector<Food> foods_;
  std::vector<Monster> monsters_;
  std::vector<FlyingFood> flyingFoods_;

  InputState previousInput_;
  GameState state_ = GameState::Playing;
  bool running_ = true;

  int tick_ = 0;
  int playerMoveCooldown_ = 0;
  std::string message_;
  std::string defeatReason_;
};

#endif
