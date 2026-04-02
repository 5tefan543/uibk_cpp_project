#pragma once

#include "game/game.hpp"
#include "input/input_state.hpp"
#include "state/state_manager.hpp"

namespace controller {

class Controller {
  private:
    StateManager stateManager;

  public:
    Controller();
    ~Controller();
    void update(const InputState &input, float dt);
    BaseState &getCurrentState();
};

} // namespace controller