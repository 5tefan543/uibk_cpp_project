#pragma once

#include "debug/debug_context.hpp"
#include "game/game.hpp"
#include "input/input_state.hpp"
#include "state/state_manager.hpp"

namespace controller {

class Controller {
  private:
    StateManager stateManager_;
    DebugContext debug_;

  public:
    Controller();
    ~Controller();
    void update(const InputState &input, float dt);
    BaseState &getCurrentState();
    DebugContext &getDebugContext();
};

} // namespace controller