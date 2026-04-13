#pragma once

#include "controller/input/input_state.hpp"
#include "controller/state/state_transition_action.hpp"
#include "controller/view/view.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/input_system.hpp"
#include "game/ecs/systems/movement_system.hpp"

namespace game {

class Game {
  private:
    Registry registry;
    InputSystem inputSystem;
    MovementSystem movementSystem;

    void initPlayer();
    void handleDebugState(controller::DebugState &debugState);

  public:
    Game();
    Game(const Game &) = delete;
    ~Game();

    controller::StateTransitionAction update(const controller::InputState &input, controller::DebugState &debugState,
                                             float dt);
    controller::View getView();
};

} // namespace game