#pragma once

#include "controller/input/input_state.hpp"
#include "controller/state/state_transition_action.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/animation_system.hpp"
#include "game/ecs/systems/camera_system.hpp"
#include "game/ecs/systems/debug_selection_system.hpp"
#include "game/ecs/systems/input_system.hpp"
#include "game/ecs/systems/movement_system.hpp"
#include "view/view.hpp"

namespace game {

class Game {
  private:
    Registry registry_;
    GameDebugSession debugSession_{registry_};

    AnimationSystem animationSystem_;
    CameraSystem cameraSystem_;
    InputSystem inputSystem_;
    MovementSystem movementSystem_;
    DebugSelectionSystem debugSelectionSystem_;

    void initWave();
    void initStage();
    void initPlayer();
    void initEnemies();
    void processDebugSession();
    void updateSystems(const controller::InputState &input, float dt);
    bool isGameOver();

  public:
    Game();
    Game(const Game &) = delete;
    ~Game();

    GameDebugSession &getDebugSession();
    bool update(const controller::InputState &input, float dt);
    void updateView(view::View &view);
};

} // namespace game