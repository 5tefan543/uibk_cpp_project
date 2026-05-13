#pragma once

#include "controller/input/input_state.hpp"
#include "controller/persistence/config_game.hpp"
#include "controller/persistence/persisted_game.hpp"
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
    controller::GameConfig config_;

    AnimationSystem animationSystem_;
    CameraSystem cameraSystem_;
    InputSystem inputSystem_;
    MovementSystem movementSystem_;
    DebugSelectionSystem debugSelectionSystem_;

    int stage_ = 1;
    int wave_ = 0;
    int score_ = 0;
    int currency_ = 0;

    void initStage();
    void initPlayer();
    void initPersistedPlayer(const controller::PersistedGame &persistedGame);
    void initEnemies();
    void processDebugSession();
    void updateSystems(const controller::InputState &input, float dt);
    bool isGameOver();

  public:
    Game();
    Game(const Game &) = delete;
    ~Game();

    GameDebugSession &getDebugSession();
    void loadFromPersistedGame(const controller::PersistedGame &persistedGame);
    controller::PersistedGame getPersistedGame() const;
    bool update(const controller::InputState &input, float dt);
    void updateView(view::View &view);
    void getNextWave();
    bool isWaveDefeated();
    void addScore(int score);
    int getScore();
    void getNextStage();
    int getWaveCount();
};

} // namespace game