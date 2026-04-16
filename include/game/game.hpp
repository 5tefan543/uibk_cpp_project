#pragma once

#include "controller/debug/debug_context.hpp"
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
    GameDebugSession debugSession{registry};

    InputSystem inputSystem;
    MovementSystem movementSystem;

    void initPlayer();
    void initDebugContext(controller::DebugContext &debug);
    void updateSystems(const controller::InputState &input, controller::DebugContext &debug, float dt);
    controller::StateTransitionAction determineStateAction(const controller::InputState &input,
                                                           controller::DebugContext &debug);
    void cleanUpDebugContext(controller::StateTransitionAction action, controller::DebugContext &debug);

  public:
    Game();
    Game(const Game &) = delete;
    ~Game();

    controller::StateTransitionAction update(const controller::InputState &input, controller::DebugContext &debug,
                                             float dt);
    controller::View getView();
};

} // namespace game