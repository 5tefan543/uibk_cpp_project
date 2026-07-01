#pragma once

#include "controller/timing.hpp"
#include "input/input_state.hpp"
#include "state/state_manager.hpp"

namespace controller {

class Controller {
  private:
    StateManager stateManager_;

  public:
    Controller();
    ~Controller();
    void update(const InputState &input, const controller::timeDelta &dt);
    BaseState &getCurrentState();
};

} // namespace controller