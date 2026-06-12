#include "Game.h"

int main() {
  Game game;
  if (!game.Initialize()) {
    return 1;
  }

  game.Run();
  return 0;
}
