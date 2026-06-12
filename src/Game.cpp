#include "Game.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <thread>
#include <vector>

static bool IsKeyDown(int virtual_key) {
    return (GetAsyncKeyState(virtual_key) & kKeyPressedMask) != 0;
}


bool Game::Initialize() {
    if (!renderer.Initialize()) {
        return false;
    }

    LoadAssets();
    Reset();
    return true;
}

void Game::Run() {
    while (running) {
        ProcessInput();

        if (state == GameState::Playing) {
            Update();
        }

        Draw();
        std::this_thread::sleep_for(std::chrono::milliseconds(kFrameDurationMs));
    }
}

void Game::Reset() {
    state = GameState::Playing;
    running = true;
    tick = 0;
    player_move_cooldown = 0;
    previous_input = InputState{};
    defeat_reason.clear();

    player.position = Vec2{};
    player.hp = kDefaultPlayerHp;
    settings = GameSettings{};

    flying_foods.clear();

    if (LoadLevelFromFile(kLevelFileName)) {
        if (state == GameState::Playing) {
            SetMessage("Level loaded. Collect food and avoid the monsters.");
        }
    } else {
        TriggerDefeat("Level file, assets, or animation data are invalid.");
    }
}

void Game::LoadAssets() {
    assets.LoadFromFile(kAssetsFileName);
}

bool Game::LoadLevelFromFile(const std::string& file_name) {
    LevelLoader loader;
    LevelData level_data;

    if (!loader.LoadFromFile(file_name, &level_data)) {
        return false;
    }

    return ApplyLevelData(level_data);
}

bool Game::ApplyLevelData(const LevelData& level_data) {
    settings = level_data.settings;
    map = level_data.map;
    foods = level_data.foods;
    player.position = level_data.player_start;
    player.hp = settings.player_hp;

    monsters.clear();
    for (const MonsterLevelSpec& spec : level_data.monsters) {
        if (!assets.HasAnimation(spec.animation_name)) {
            TriggerDefeat("Missing animation in assets.txt: " + spec.animation_name);
            return false;
        }

        std::vector<std::string> frame_names = assets.GetAnimationFrames(spec.animation_name);
        if (frame_names.empty()) {
            TriggerDefeat("Animation has no frames: " + spec.animation_name);
            return false;
        }

        for (const std::string& frame_name : frame_names) {
            if (!assets.HasSprite(frame_name)) {
                TriggerDefeat("Missing sprite frame in assets.txt: " + frame_name);
                return false;
            }
        }

        Monster monster;
        monster.name = spec.name;
        monster.map_symbol = spec.map_symbol;
        monster.position = spec.position;
        monster.hunger_limit = spec.hunger_limit;
        monster.damage = spec.damage;
        monster.move_delay = spec.move_delay;
        monster.frame_delay = spec.frame_delay;

        for (FoodType type : spec.needs) {
            monster.needs.push_back(FoodNeed{type});
        }

        monster.frame_names = frame_names;
        monsters.push_back(monster);
    }

    if (monsters.empty()) {
        TriggerDefeat("No monsters were loaded from level.txt.");
        return false;
    }

    return true;
}

void Game::ProcessInput() {
    const InputState input = ReadInput();

    if (player_move_cooldown > 0) {
        --player_move_cooldown;
    }

    if (JustPressed(input.q, previous_input.q)) {
        running = false;
    }

    if (state != GameState::Playing) {
        if (JustPressed(input.r, previous_input.r)) {
            Reset();
        }
        previous_input = input;
        return;
    }

    TryHandlePlayerMovement(input);

    if (JustPressed(input.e, previous_input.e)) {
        SetMessage("Food is sent automatically when you step on it. Hold movement keys to run.");
    }

    previous_input = input;
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
    if (player_move_cooldown > 0) {
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
    player_move_cooldown = settings.player_move_delay;
}

void Game::TryMovePlayer(int dx, int dy) {
    const int next_x = player.position.x + dx;
    const int next_y = player.position.y + dy;

    if (IsWall(next_x, next_y)) {
        SetMessage("A wall blocks the path.");
        return;
    }

    if (IsMonsterAt(next_x, next_y, -1)) {
        SetMessage("A monster blocks the path.");
        return;
    }

    player.position.x = next_x;
    player.position.y = next_y;

    CheckPlayerNearMonsterDefeat();
    if (state == GameState::Playing) {
        CheckFoodPickup();
    }
}

bool Game::IsWall(int x, int y) const {
    if (!IsInsideMap(x, y)) {
        return true;
    }
    return map[y][x] == kWallCell;
}

bool Game::IsBlockedForMonster(int x, int y, int moving_monster_index) const {
    if (IsWall(x, y)) {
        return true;
    }

    if (player.position.x == x && player.position.y == y) {
        return true;
    }

    return IsMonsterAt(x, y, moving_monster_index);
}

bool Game::IsMonsterAt(int x, int y, int ignored_monster_index) const {
    for (int i = 0; i < static_cast<int>(monsters.size()); ++i) {
        if (i == ignored_monster_index) {
            continue;
        }

        if (monsters[i].position.x == x && monsters[i].position.y == y) {
            return true;
        }
    }

    return false;
}

bool Game::IsInsideMap(int x, int y) const {
    return y >= 0 && y < static_cast<int>(map.size()) &&
           x >= 0 && x < static_cast<int>(map[y].size());
}

void Game::Update() {
    ++tick;
    UpdateMonsterAnimations();

    CheckPlayerNearMonsterDefeat();
    if (state != GameState::Playing) {
        return;
    }

    UpdateMonsterMovement();
    CheckPlayerNearMonsterDefeat();
    if (state != GameState::Playing) {
        return;
    }

    UpdateHungerAndDamage();
    if (state != GameState::Playing) {
        return;
    }

    UpdateFlyingFood();
    if (state != GameState::Playing) {
        return;
    }

    CheckWinCondition();
}

void Game::UpdateMonsterAnimations() {
    for (Monster& monster : monsters) {
        if (monster.frame_names.empty()) {
            continue;
        }

        ++monster.animation_timer;
        if (monster.animation_timer >= monster.frame_delay) {
            monster.animation_timer = 0;
            monster.current_frame = (monster.current_frame + 1) % static_cast<int>(monster.frame_names.size());
        }
    }
}

void Game::UpdateMonsterMovement() {
    for (int i = 0; i < static_cast<int>(monsters.size()); ++i) {
        Monster& monster = monsters[i];
        if (monster.IsFed()) {
            continue;
        }

        ++monster.move_timer;
        const int delay = monster.aggressive ? std::max(settings.min_aggressive_move_delay, monster.move_delay / 2) : monster.move_delay;
        if (monster.move_timer < delay) {
            continue;
        }
        monster.move_timer = 0;

        if (IsAdjacent(monster.position, player.position)) {
            continue;
        }

        const int dx = Sign(player.position.x - monster.position.x);
        const int dy = Sign(player.position.y - monster.position.y);
        const int abs_dx = std::abs(player.position.x - monster.position.x);
        const int abs_dy = std::abs(player.position.y - monster.position.y);

        std::vector<Vec2> candidates;
        if (abs_dx >= abs_dy) {
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

            const int next_x = monster.position.x + candidate.x;
            const int next_y = monster.position.y + candidate.y;
            if (!IsBlockedForMonster(next_x, next_y, i)) {
                monster.position.x = next_x;
                monster.position.y = next_y;
                break;
            }
        }
    }
}

void Game::UpdateHungerAndDamage() {
    if (settings.hunger_update_interval <= 0 ||
        tick % settings.hunger_update_interval != 0) {
        return;
    }

    for (Monster& monster : monsters) {
        if (monster.IsFed()) {
            monster.hunger = 0;
            monster.aggressive = false;
            continue;
        }

        monster.hunger += settings.hunger_per_update;
        if (monster.hunger >= monster.hunger_limit) {
            monster.hunger = monster.hunger_limit;
            monster.aggressive = true;
        }
    }
}

void Game::UpdateFlyingFood() {
    for (FlyingFood& flying_food : flying_foods) {
        if (!flying_food.active) {
            continue;
        }

        if (flying_food.target_monster_index >= 0 &&
            flying_food.target_monster_index < static_cast<int>(monsters.size())) {
            flying_food.target_x = static_cast<float>(monsters[flying_food.target_monster_index].position.x);
            flying_food.target_y = static_cast<float>(monsters[flying_food.target_monster_index].position.y);
        }

        const float dx = flying_food.target_x - flying_food.x;
        const float dy = flying_food.target_y - flying_food.y;
        const float distance = std::sqrt(dx * dx + dy * dy);

        if (distance <= flying_food.speed || distance < kFoodDeliveryDistanceEpsilon) {
            flying_food.x = flying_food.target_x;
            flying_food.y = flying_food.target_y;
            const FlyingFood delivered_food = flying_food;
            flying_food.active = false;
            CompleteFoodDelivery(delivered_food);
            continue;
        }

        flying_food.x += (dx / distance) * flying_food.speed;
        flying_food.y += (dy / distance) * flying_food.speed;
    }

    flying_foods.erase(
        std::remove_if(flying_foods.begin(), flying_foods.end(),
                       [](const FlyingFood& food) { return !food.active; }),
        flying_foods.end());
}

void Game::CheckFoodPickup() {
    for (Food& food : foods) {
        if (!food.active) {
            continue;
        }

        if (food.position.x == player.position.x && food.position.y == player.position.y) {
            StartFoodFlight(food);
            return;
        }
    }
}

void Game::CheckWinCondition() {
    if (AllMonstersFed()) {
        state = GameState::Victory;
        SetMessage("All monsters are fed. Balance is restored.");
    }
}

void Game::StartFoodFlight(Food& food) {
    food.active = false;

    int target_monster_index = FindCorrectTargetMonster(food.type, player.position);
    bool correct_food = true;
    int target_need_index = -1;

    if (target_monster_index >= 0) {
        target_need_index = ReserveCorrectMonsterNeed(target_monster_index, food.type);
    } else {
        correct_food = false;
        target_monster_index = FindHungriestMonster();
        if (target_monster_index >= 0) {
            target_need_index = ReserveWrongMonsterNeed(target_monster_index, food.type);
        }
    }

    if (target_monster_index < 0) {
        SetMessage("No monster can receive this food now.");
        return;
    }

    FlyingFood flying_food;
    flying_food.type = food.type;
    flying_food.symbol = food.Symbol();
    flying_food.x = static_cast<float>(player.position.x);
    flying_food.y = static_cast<float>(player.position.y);
    flying_food.target_x = static_cast<float>(monsters[target_monster_index].position.x);
    flying_food.target_y = static_cast<float>(monsters[target_monster_index].position.y);
    flying_food.target_monster_index = target_monster_index;
    flying_food.target_need_index = target_need_index;
    flying_food.correct_food = correct_food;
    flying_food.speed = settings.flying_food_speed;
    flying_food.active = true;

    flying_foods.push_back(flying_food);

    if (correct_food) {
        SetMessage(std::string("Food ") + food.Symbol() + " flies to " +
                   monsters[target_monster_index].name + ".");
    } else {
        SetMessage(std::string("Wrong food ") + food.Symbol() + " flies to the hungriest monster: " +
                   monsters[target_monster_index].name + ".");
    }
}

int Game::FindCorrectTargetMonster(FoodType type, Vec2 start) const {
    int best_index = -1;
    int best_distance = std::numeric_limits<int>::max();

    for (int i = 0; i < static_cast<int>(monsters.size()); ++i) {
        if (!monsters[i].NeedsFood(type)) {
            continue;
        }

        const int distance = DistanceSquared(start, monsters[i].position);
        if (distance < best_distance) {
            best_distance = distance;
            best_index = i;
        }
    }

    return best_index;
}

int Game::FindHungriestMonster() const {
    int best_index = -1;
    int best_hunger = -1;
    int best_active_cells = -1;

    for (int i = 0; i < static_cast<int>(monsters.size()); ++i) {
        const Monster& monster = monsters[i];
        if (monster.IsFed()) {
            continue;
        }

        const int active_cells = monster.ActiveNeedCount();
        if (monster.hunger > best_hunger ||
            (monster.hunger == best_hunger && active_cells > best_active_cells)) {
            best_hunger = monster.hunger;
            best_active_cells = active_cells;
            best_index = i;
        }
    }

    return best_index;
}

int Game::ReserveCorrectMonsterNeed(int monster_index, FoodType type) {
    if (monster_index < 0 || monster_index >= static_cast<int>(monsters.size())) {
        return -1;
    }

    Monster& monster = monsters[monster_index];
    for (int i = 0; i < static_cast<int>(monster.needs.size()); ++i) {
        FoodNeed& need = monster.needs[i];
        if (need.type == type && need.state == NeedState::Missing) {
            need.state = NeedState::Flying;
            need.delivered_type = type;
            return i;
        }
    }

    return -1;
}

int Game::ReserveWrongMonsterNeed(int monster_index, FoodType type) {
    if (monster_index < 0 || monster_index >= static_cast<int>(monsters.size())) {
        return -1;
    }

    Monster& monster = monsters[monster_index];
    const int missing_index = monster.FirstMissingNeedIndex();
    if (missing_index >= 0) {
        monster.needs[missing_index].state = NeedState::WrongFlying;
        monster.needs[missing_index].delivered_type = type;
    }

    return missing_index;
}

void Game::CompleteFoodDelivery(const FlyingFood& flying_food) {
    if (flying_food.correct_food) {
        CompleteCorrectMonsterNeed(flying_food.target_monster_index,
                                   flying_food.target_need_index,
                                   flying_food.type);
    } else {
        CompleteWrongMonsterNeed(flying_food.target_monster_index,
                                 flying_food.target_need_index,
                                 flying_food.type);
    }
}

void Game::CompleteCorrectMonsterNeed(int monster_index, int need_index, FoodType type) {
    if (monster_index < 0 || monster_index >= static_cast<int>(monsters.size())) {
        return;
    }

    Monster& monster = monsters[monster_index];
    if (need_index >= 0 && need_index < static_cast<int>(monster.needs.size())) {
        FoodNeed& need = monster.needs[need_index];
        if (need.type == type && need.state == NeedState::Flying) {
            need.state = NeedState::Eaten;
            need.delivered_type = type;
            monster.hunger = std::max(0, monster.hunger - settings.correct_food_hunger_reduction);
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

void Game::CompleteWrongMonsterNeed(int monster_index, int need_index, FoodType type) {
    if (monster_index < 0 || monster_index >= static_cast<int>(monsters.size())) {
        return;
    }

    Monster& monster = monsters[monster_index];
    const bool was_full_and_starving = monster.HasAllNeedCellsActive() && monster.IsFullyHungry();

    if (was_full_and_starving) {
        TriggerDefeat(monster.name + " was fully hungry and rejected wrong food.");
        return;
    }

    if (need_index >= 0 && need_index < static_cast<int>(monster.needs.size())) {
        FoodNeed& need = monster.needs[need_index];
        if (need.state == NeedState::WrongFlying) {
            need.state = NeedState::Wrong;
            need.delivered_type = type;
        }
    }

    monster.hunger += settings.wrong_food_hunger_increase;
    if (monster.hunger >= monster.hunger_limit) {
        monster.hunger = monster.hunger_limit;
        monster.aggressive = true;
    }

    if (need_index >= 0) {
        SetMessage(monster.name + " received wrong " + FoodTypeToName(type) +
                   ". One hunger cell is activated.");
    } else {
        SetMessage(monster.name + " received wrong " + FoodTypeToName(type) +
                   ". All hunger cells are already active.");
    }
}

void Game::TriggerDefeat(const std::string& reason) {
    defeat_reason = reason;
    player.hp = 0;
    state = GameState::Defeat;
    SetMessage(reason);
}

void Game::CheckPlayerNearMonsterDefeat() {
    if (state != GameState::Playing) {
        return;
    }

    for (const Monster& monster : monsters) {
        if (monster.IsFed()) {
            continue;
        }

        if (IsAdjacent(player.position, monster.position)) {
            TriggerDefeat(monster.name + " got too close to the player.");
            return;
        }
    }
}

bool Game::AllMonstersFed() const {
    for (const Monster& monster : monsters) {
        if (!monster.IsFed()) {
            return false;
        }
    }
    return true;
}

void Game::Draw() {
    renderer.Clear();

    if (state == GameState::Victory) {
        DrawEndScreen("VICTORY", "All Ling Shou creatures are fed.");
        renderer.Present();
        return;
    }

    if (state == GameState::Defeat) {
        const std::string subtitle = defeat_reason.empty() ?
            "The hungry spirits overcame you." : defeat_reason;
        DrawEndScreen("DEFEAT", subtitle);
        renderer.Present();
        return;
    }

    DrawMap();
    DrawMonsterPanels();
    DrawHud();
    renderer.Present();
}

void Game::DrawMap() {
    const int map_height = static_cast<int>(map.size());
    const int map_width = map_height > 0 ? static_cast<int>(map[0].size()) : kScreenWidth;

    renderer.DrawBox(kMapOriginX - 1, kMapOriginY - 1, map_width + 2, map_height + 2);
    renderer.DrawText(kMapOriginX, kMapOriginY - 1, " World ");

    for (int y = 0; y < static_cast<int>(map.size()); ++y) {
        for (int x = 0; x < static_cast<int>(map[y].size()); ++x) {
            renderer.DrawChar(kMapOriginX + x, kMapOriginY + y, map[y][x]);
        }
    }

    for (const Food& food : foods) {
        if (food.active) {
            renderer.DrawChar(kMapOriginX + food.position.x, kMapOriginY + food.position.y, food.Symbol());
        }
    }

    for (const Monster& monster : monsters) {
        renderer.DrawChar(kMapOriginX + monster.position.x, kMapOriginY + monster.position.y, monster.map_symbol);
    }

    for (const FlyingFood& food : flying_foods) {
        if (food.active) {
            const int x = static_cast<int>(std::round(food.x));
            const int y = static_cast<int>(std::round(food.y));
            renderer.DrawChar(kMapOriginX + x, kMapOriginY + y, food.symbol);
        }
    }

    renderer.DrawChar(kMapOriginX + player.position.x, kMapOriginY + player.position.y, kPlayerSymbol);
}

void Game::DrawHud() {
    const int hud_y = kMapOriginY + static_cast<int>(map.size()) + kHudOffsetY;
    renderer.DrawBox(kHudBoxX, hud_y - 1, kHudBoxWidth, kHudBoxHeight);
    renderer.DrawText(kHudTextX, hud_y, "HP: " + std::to_string(player.hp));
    renderer.DrawText(kHudControlsX, hud_y, "Controls: hold WASD/arrows to move | E info | R restart | Q/Esc quit");
    renderer.DrawText(kHudTextX, hud_y + 2, "Legend: P player | @ % ^ food | monster symbols are loaded from level.txt | # wall");
    renderer.DrawText(kHudTextX, hud_y + 4, "Message: " + message.substr(0, kHudMessageLimit));
}

void Game::DrawMonsterPanels() {
    renderer.DrawBox(kPanelOriginX - 1, kPanelOriginY - 1, kPanelWidth, kPanelHeight);
    renderer.DrawText(kPanelOriginX, kPanelOriginY - 1, " Monsters ");

    for (int i = 0; i < static_cast<int>(monsters.size()); ++i) {
        const Monster& monster = monsters[i];
        const int base_y = kPanelOriginY + i * kMonsterPanelStepY;

        renderer.DrawText(kPanelOriginX, base_y, monster.name + " [" + monster.map_symbol + "]");
        DrawNeedBar(monster, kPanelOriginX, base_y + 1);

        std::string state = "Chasing";
        if (monster.IsFed()) {
            state = "Fed";
        } else if (monster.aggressive) {
            state = "Aggressive";
        }
        renderer.DrawText(kPanelOriginX, base_y + 2,
                           "State: " + state + " | Hunger: " + std::to_string(monster.hunger) + "/" +
                               std::to_string(monster.hunger_limit));

        if (!monster.frame_names.empty()) {
            const std::string& frame_name = monster.frame_names[monster.current_frame];
            renderer.DrawSprite(kPanelOriginX, base_y + 4, assets.GetSprite(frame_name));
        }
    }
}

void Game::DrawNeedBar(const Monster& monster, int x, int y) {
    renderer.DrawText(x, y, "Needs: ");

    int cursor_x = x + 7;
    for (const FoodNeed& need : monster.needs) {
        char ch = FoodTypeToChar(need.type);
        if (need.state == NeedState::Flying || need.state == NeedState::WrongFlying) {
            ch = kNeedFlyingSymbol;
        } else if (need.state == NeedState::Eaten) {
            ch = kNeedEatenSymbol;
        } else if (need.state == NeedState::Wrong) {
            ch = FoodTypeToChar(need.delivered_type);
        }

        renderer.DrawChar(cursor_x, y, '[');
        renderer.DrawChar(cursor_x + 1, y, ch);
        renderer.DrawChar(cursor_x + 2, y, ']');
        cursor_x += 4;
    }
}

void Game::DrawEndScreen(const std::string& title, const std::string& subtitle) {
    const int x = (ConsoleRenderer::kWidth - kEndBoxWidth) / 2;
    const int y = (ConsoleRenderer::kHeight - kEndBoxHeight) / 2;

    renderer.DrawBox(x, y, kEndBoxWidth, kEndBoxHeight);
    renderer.DrawText(x + (kEndBoxWidth - static_cast<int>(title.size())) / 2,
                      y + kEndTitleOffsetY, title);
    renderer.DrawText(x + (kEndBoxWidth - static_cast<int>(subtitle.size())) / 2,
                      y + kEndSubtitleOffsetY, subtitle);

    if (title == "VICTORY") {
        renderer.DrawText(x + kEndVictoryTextX, y + kEndVictoryFirstTextY,
                          "Balance is restored. The spirits are calm.");
        renderer.DrawText(x + kEndVictorySecondTextX, y + kEndVictorySecondTextY,
                          "All monsters are fed.");
    } else {
        renderer.DrawText(x + kEndDefeatTextX, y + kEndDefeatTextY,
                          "Try again and feed them faster.");
    }

    renderer.DrawText(x + kEndPromptTextX, y + kEndBoxHeight - kEndPromptOffsetFromBottom,
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
    return dx <= kAdjacentDistance && dy <= kAdjacentDistance;
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

void Game::SetMessage(const std::string& new_message) {
    message = new_message;
}
