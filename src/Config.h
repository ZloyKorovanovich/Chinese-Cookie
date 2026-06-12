#ifndef CONFIG_H_
#define CONFIG_H_

inline constexpr char kAssetsFileName[] = "assets.txt";
inline constexpr char kLevelFileName[] = "level.txt";

inline constexpr int kScreenWidth = 120;
inline constexpr int kScreenHeight = 40;
inline constexpr int kFrameDurationMs = 50;
inline constexpr unsigned int kConsoleCodePage = 437;
inline constexpr int kKeyPressedMask = 0x8000;

inline constexpr int kMapOriginX = 2;
inline constexpr int kMapOriginY = 2;
inline constexpr int kPanelOriginX = 64;
inline constexpr int kPanelOriginY = 2;
inline constexpr int kPanelWidth = 54;
inline constexpr int kPanelHeight = 23;
inline constexpr int kMonsterPanelStepY = 11;

inline constexpr int kHudBoxX = 1;
inline constexpr int kHudBoxWidth = 118;
inline constexpr int kHudBoxHeight = 8;
inline constexpr int kHudTextX = 3;
inline constexpr int kHudControlsX = 15;
inline constexpr int kHudOffsetY = 3;
inline constexpr int kHudMessageLimit = 100;

inline constexpr int kEndBoxWidth = 68;
inline constexpr int kEndBoxHeight = 13;
inline constexpr int kEndTitleOffsetY = 2;
inline constexpr int kEndSubtitleOffsetY = 5;
inline constexpr int kEndVictoryTextX = 13;
inline constexpr int kEndVictorySecondTextX = 18;
inline constexpr int kEndDefeatTextX = 18;
inline constexpr int kEndPromptTextX = 18;
inline constexpr int kEndVictoryFirstTextY = 7;
inline constexpr int kEndVictorySecondTextY = 9;
inline constexpr int kEndDefeatTextY = 8;
inline constexpr int kEndPromptOffsetFromBottom = 2;

inline constexpr char kWallCell = '#';
inline constexpr char kFloorCell = '.';
inline constexpr char kPlayerSymbol = 'P';
inline constexpr char kNeedFlyingSymbol = '*';
inline constexpr char kNeedEatenSymbol = '_';
inline constexpr char kMissingFoodSymbol = '?';

inline constexpr char kFoodAtSymbol = '@';
inline constexpr char kFoodPercentSymbol = '%';
inline constexpr char kFoodCaretSymbol = '^';

inline constexpr int kDefaultPlayerHp = 100;
inline constexpr int kDefaultPlayerMoveDelay = 3;
inline constexpr int kDefaultHungerLimit = 100;
inline constexpr int kDefaultMonsterDamage = 4;
inline constexpr int kDefaultMonsterMoveDelay = 8;
inline constexpr int kDefaultMonsterFrameDelay = 8;
inline constexpr int kDefaultHungerUpdateInterval = 20;
inline constexpr int kDefaultHungerPerUpdate = 3;
inline constexpr int kDefaultCorrectFoodHungerReduction = 35;
inline constexpr int kDefaultWrongFoodHungerIncrease = 35;
inline constexpr int kDefaultMinAggressiveMoveDelay = 3;
inline constexpr float kDefaultFlyingFoodSpeed = 0.65f;

inline constexpr int kAdjacentDistance = 1;
inline constexpr float kFoodDeliveryDistanceEpsilon = 0.05f;

#endif  // CONFIG_H_
