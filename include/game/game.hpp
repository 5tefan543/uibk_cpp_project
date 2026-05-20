#pragma once

#include "controller/input/input_state.hpp"
#include "controller/persistence/config_game.hpp"
#include "controller/persistence/persisted_game.hpp"
#include "controller/state/state_transition_action.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/animation_system.hpp"
#include "game/ecs/systems/camera_system.hpp"
#include "game/ecs/systems/collision_detection_system.hpp"
#include "game/ecs/systems/debug_selection_system.hpp"
#include "game/ecs/systems/input_system.hpp"
#include "game/ecs/systems/movement_system.hpp"
#include "view/view.hpp"

namespace game {

class Game {
  private:
    Registry registry_;
    GameDebugSession debugSession_{registry_};
    controller::GameConfig config_;
    float currentWaveDuration_;

    AnimationSystem animationSystem_;
    CameraSystem cameraSystem_;
    InputSystem inputSystem_;
    MovementSystem movementSystem_;
    DebugSelectionSystem debugSelectionSystem_;
    CollisionDetectionSystem collisionDetectionSystem_;

    int stage_ = 1;
    int wave_ = 1;
    int score_ = 0;
    int currency_ = 0;
    // We need to store view::Text as a member because ViewElement stores a reference to it,
    // so we must ensure that the referenced object lives long enough.
    //
    // Is this approach ok?
    // Or would it make more sense to only store certain elements like view::Card and
    // view::Button as reference_wrapped, while keeping view::Text as a value type in ViewElement?
    //
    // At the moment view::Sprite is NOT reference_wrapped, so ViewElement already contains
    // a mix of reference_wrapped and value types.
    //
    // If we changed view::Sprite to also be reference_wrapped, we would need to store
    // all sprites in a deque inside Game.
    view::Text stageWaveInfo_;

    explicit Game(int wave);

    void initMap();
    void initCamera();
    void initPlayer();
    void initWave(int waveNumber);
    void initEnemies();
    void processDebugSession(float dt);
    void updateSystems(const controller::InputState &input, float dt);
    bool isWaveFinished();
    void addScore(int score);

  public:
    Game();
    Game(const controller::PersistedGame &persistedGame);
    Game(const Game &) = delete;
    Game(Game &&) = delete;
    ~Game();

    GameDebugSession &getDebugSession();
    controller::PersistedGame getPersistedGame() const;
    controller::StateTransitionAction update(const controller::InputState &input, float dt);
    bool isGameOver();
    void updateView(view::View &view);
};

} // namespace game