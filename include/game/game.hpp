#pragma once
#include "game_state.hpp"

namespace game {

class Game {
  private:
    GameState gameState;

  public:
    Game();
    ~Game();
};

} // namespace game