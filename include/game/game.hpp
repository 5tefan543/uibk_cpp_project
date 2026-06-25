#pragma once

#include "config/game_config.hpp"
#include "controller/input/input_state.hpp"
#include "controller/state/state_transition_action.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/animation_system.hpp"
#include "game/ecs/systems/camera_system.hpp"
#include "game/ecs/systems/collision_detection_system.hpp"
#include "game/ecs/systems/damage_system.hpp"
#include "game/ecs/systems/debug_selection_system.hpp"
#include "game/ecs/systems/enemy_ai_system.hpp"
#include "game/ecs/systems/input_system.hpp"
#include "game/ecs/systems/movement_system.hpp"
#include "game/ecs/systems/sound_system.hpp"
#include "game/ecs/systems/spawn_enemy_system.hpp"
#include "game/location_table.hpp"
#include "game/persisted_game.hpp"
#include "view/view.hpp"

namespace game {

class Game {
  private:
    Registry registry_;
    config::GameConfig config_;
    LocationTable locationTable_;
    GameDebugSession debugSession_{registry_, locationTable_};
    float currentWaveDuration_;
    bool shouldOpenStore_ = false;

    AnimationSystem animationSystem_;
    CameraSystem cameraSystem_;
    InputSystem inputSystem_;
    MovementSystem movementSystem_;
    DebugSelectionSystem debugSelectionSystem_;
    EnemyAI enemyAI_;
    SpawnEnemySystem spawnEnemySystem_;
    CollisionDetectionSystem collisionDetectionSystem_;
    DamageSystem damageSystem_;
    SoundSystem soundSystem_;

    int stage_ = 1;
    int wave_ = 1;
    view::Text stageWaveInfo_;

    explicit Game(int wave, CharacterType characterType);

    void initMap();
    void switchMap();
    void cleanup();
    void initCamera(Position position);
    void initPlayer(CharacterType characterType);
    void initPlayer(Position position, PlayerStats playerStats);
    void initWave(int waveNumber);
    void processDebugSession(float dt);
    void updateSystems(const controller::InputState &input, float dt);
    void addScore(int score);
    bool isWaveFinished();

  public:
    Game();
    explicit Game(CharacterType characterType);
    explicit Game(const PersistedGame &persistedGame);
    Game(const Game &) = delete;
    Game(Game &&) = delete;
    ~Game();

    GameDebugSession &getDebugSession();
    PersistedGame getPersistedGame() const;
    PlayerStats &getPlayerStats();
    controller::StateTransitionAction update(const controller::InputState &input, float dt);
    bool isGameOver();
    int getWaveNumber();
    void save();
    void setShouldOpenStore(bool shouldOpenStore);
    void updateView(view::View &view);
};

} // namespace game