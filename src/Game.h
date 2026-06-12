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
    enum class GameState {
        Playing,
        Victory,
        Defeat
    };

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
    bool LoadLevelFromFile(const std::string& file_name);
    bool ApplyLevelData(const LevelData& level_data);

    void ProcessInput();
    InputState ReadInput() const;
    bool JustPressed(bool current, bool previous) const;

    void TryHandlePlayerMovement(const InputState& input);
    void TryMovePlayer(int dx, int dy);
    bool IsWall(int x, int y) const;
    bool IsBlockedForMonster(int x, int y, int moving_monster_index) const;
    bool IsMonsterAt(int x, int y, int ignored_monster_index) const;
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
    int ReserveCorrectMonsterNeed(int monster_index, FoodType type);
    int ReserveWrongMonsterNeed(int monster_index, FoodType type);
    void CompleteFoodDelivery(const FlyingFood& flying_food);
    void CompleteCorrectMonsterNeed(int monster_index, int need_index, FoodType type);
    void CompleteWrongMonsterNeed(int monster_index, int need_index, FoodType type);
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
    void SetMessage(const std::string& new_message);

    ConsoleRenderer renderer;
    AssetManager assets;

    Player player;
    std::vector<std::string> map;
    std::vector<Food> foods;
    std::vector<Monster> monsters;
    std::vector<FlyingFood> flying_foods;

    InputState previous_input;
    GameState state = GameState::Playing;
    bool running = true;

    int tick = 0;
    int player_move_cooldown = 0;
    GameSettings settings;
    std::string message;
    std::string defeat_reason;
};

#endif  // GAME_H_
