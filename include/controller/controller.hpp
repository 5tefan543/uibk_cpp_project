#pragma once
#include "game/game.hpp"
#include "input_state.hpp"

namespace controller {

class Controller {
  private:
    game::Game game;

  public:
    Controller();
    ~Controller();
    void handleInput(const InputState &input);
    void update(float dt);
};

} // namespace controller