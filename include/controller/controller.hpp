#pragma once

#include "game/game.hpp"
#include "input/input_state.hpp"
#include "state/state_stack.hpp"

namespace controller {

class Controller {
  private:
    StateStack states;

  public:
    Controller();
    ~Controller();
    void update(const InputState &input, float dt);
};

} // namespace controller