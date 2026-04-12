#pragma once

#include "debug/debug_state.hpp"
#include "game/game.hpp"
#include "input/input_state.hpp"
#include "state/state_manager.hpp"

namespace controller {

class Controller {
  private:
    StateManager stateManager;
    DebugState debugState;

  public:
    Controller();
    ~Controller();
    void update(const InputState &input, float dt);
    BaseState &getCurrentState();
    DebugState &getDebugState();
};

} // namespace controller