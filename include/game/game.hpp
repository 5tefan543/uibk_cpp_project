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
    Registry registry_;
    GameDebugSession debugSession_{registry_};

    InputSystem inputSystem_;
    MovementSystem movementSystem_;

    void initPlayer();
    void processDebugSession(controller::DebugContext &debug);
    void updateSystems(const controller::InputState &input, controller::DebugContext &debug, float dt);
    bool isGameOver();

  public:
    Game();
    Game(const Game &) = delete;
    ~Game();

    GameDebugSession &getDebugSession();
    bool update(const controller::InputState &input, controller::DebugContext &debug, float dt);
    controller::View getView();
};

} // namespace game