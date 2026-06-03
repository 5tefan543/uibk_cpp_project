#include "game/game.hpp"
#include "controller/debug/debug_context.hpp"
#include "controller/persistence/persisted_game.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "controller/state/state_transition_action.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/camera_tag.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "logging/log.hpp"
#include "view/sprite.hpp"
#include <format>
#include <view/text.hpp>

namespace game {

Game::Game(int wave) : locationTable_({40, 40}) // 1920/40 = 48 buckets; 1080/40 = 27 buckets
{
    config_ = controller::PersistenceManager::getConfig();
    initMap();
    initCamera();
    initPlayer();
    initWave(wave);
}

Game::Game() : Game(1)
{
    logger::log(logger::DEBUG, "New game constructed");
}

Game::Game(const controller::PersistedGame &persistedGame) : Game(persistedGame.wave)
{
    logger::log(logger::DEBUG, "Game constructed from persisted game");

    score_ = persistedGame.score;
    currency_ = persistedGame.currency;

    auto players = registry_.view<PlayerTag>();
    if (!players.empty()) {
        Position &position = registry_.getComponent<Position>(players.front());

        position.x = persistedGame.playerStats.posX;
        position.y = persistedGame.playerStats.posY;

        PlayerStats &playerStats = registry_.getComponent<PlayerStats>(players.front());
        playerStats.maxHealth = persistedGame.playerStats.maxHealth;
        playerStats.health = persistedGame.playerStats.maxHealth;
        playerStats.attackPower = persistedGame.playerStats.attackPower;
        playerStats.attackSpeed = persistedGame.playerStats.attackSpeed;
        playerStats.defense = persistedGame.playerStats.defense;
        playerStats.moveSpeed = persistedGame.playerStats.speed;
        playerStats.hasDash = persistedGame.playerStats.hasDash;
    }
}

Game::~Game()
{
    logger::log(logger::DEBUG, "Game destructed");
}

void Game::initMap()
{
    // Initialize map and camera
    Entity map = registry_.createEntity();
    registry_.addComponent<MapTag>(map, {});
    registry_.addComponent<Position>(map, {0.0f, 0.0f});
    view::Sprite mapSprite = {
        .imagePath = "assets/maps/map.bmp",
        .width = 1920.0f * 2.0f,
        .height = 1080.0f * 2.0f,
    };
    registry_.addComponent<view::Sprite>(map, mapSprite);
}

void Game::initCamera()
{
    Entity camera = registry_.createEntity();
    registry_.addComponent<CameraTag>(camera, {});
    registry_.addComponent<Position>(camera, {0.0f, 0.0f});
}

void Game::initPlayer()
{
    Entity player = registry_.createEntity();
    registry_.addComponent<PlayerTag>(player, {});

    PlayerStats playerStats;
    playerStats.maxHealth = 100.0f;
    playerStats.health = playerStats.maxHealth;
    playerStats.attackPower = 10.0f;
    playerStats.attackSpeed = 1.0f;
    playerStats.defense = 0.0f;
    playerStats.moveSpeed = 750.0f;
    playerStats.hasDash = false;
    registry_.addComponent<PlayerStats>(player, playerStats);

    registry_.addComponent<Position>(player, {100.0f, 100.0f});
    registry_.addComponent<Velocity>(player, {0.0f, 0.0f});
    Animation playerAnimation = {
        .baseTexturePath = "assets/characters/character_",
    };
    registry_.addComponent<Animation>(player, playerAnimation);
    registry_.addComponent<view::Sprite>(player, {.imagePath = playerAnimation.baseTexturePath + "right_1.png"});
}

void Game::initWave(int waveNumber)
{
    currentWaveDuration_ = 0.0f;
    wave_ = waveNumber;
    debugSession_.wave = waveNumber;

    stage_ = ((wave_ - 1) / config_.wavesPerStage) + 1;
    debugSession_.stage = stage_;

    if (wave_ > 1) {
        auto gameSave = getPersistedGame();
        if (!controller::PersistenceManager::saveGame(gameSave)) {

            // TODO error via gui not console
        }
    }

    spawnEnemySystem_.update(registry_, wave_, config_);
    logger::log(logger::DEBUG, std::format("Starting wave {} of stage {}", wave_, stage_));
}

GameDebugSession &Game::getDebugSession()
{
    return debugSession_;
}

controller::PersistedGame Game::getPersistedGame() const
{
    controller::PersistedGame persistedGame;
    persistedGame.wave = wave_;
    persistedGame.score = score_;
    persistedGame.currency = currency_;

    auto players = registry_.view<Position, PlayerStats, PlayerTag>();
    if (!players.empty()) {
        const Position &position = registry_.getComponent<Position>(players.front());
        persistedGame.playerStats.posX = position.x;
        persistedGame.playerStats.posY = position.y;

        const PlayerStats &playerStats = registry_.getComponent<PlayerStats>(players.front());
        persistedGame.playerStats.maxHealth = playerStats.maxHealth;
        persistedGame.playerStats.attackPower = playerStats.attackPower;
        persistedGame.playerStats.attackSpeed = playerStats.attackSpeed;
        persistedGame.playerStats.defense = playerStats.defense;
        persistedGame.playerStats.speed = playerStats.moveSpeed;
        persistedGame.playerStats.hasDash = playerStats.hasDash;
    }

    return persistedGame;
}

controller::StateTransitionAction Game::update(const controller::InputState &input, float dt)
{
    processDebugSession(dt);
    updateSystems(input, dt);

    // update clock
    currentWaveDuration_ += dt;

    if (isWaveFinished()) {
        addScore(config_.waveDurationSeconds - (int)currentWaveDuration_);

        bool shouldOpenStore = (wave_ % config_.wavesPerStage) == 0;
        initWave(++wave_);

        if (shouldOpenStore) {
            return controller::StateTransitionAction::PushProgressionStore;
        }

        return controller::StateTransitionAction::None;
    }

    if (isGameOver()) {
        controller::PersistenceManager::deleteSave();
        return controller::StateTransitionAction::ReplaceCurrentWithGameOverMenu;
    }

    return controller::StateTransitionAction::None;
}

void Game::processDebugSession(float dt)
{
    controller::DebugContext &debug = controller::DebugContext::get();

    if (!debug.active) {
        return;
    }

    if (debugSession_.isClockPaused) {
        currentWaveDuration_ -= dt;
    }

    // Handle stage/wave reload request
    if (debugSession_.isStageWaveReloadRequested) {
        debugSession_.isStageWaveReloadRequested = false;
        initWave(debugSession_.wave);
    }

    // Handle player destruction request
    if (debugSession_.isPlayerDestructionRequested) {
        debugSession_.isPlayerDestructionRequested = false;
        logger::log(logger::DEBUG, "Destroying player entity!");

        for (Entity player : registry_.view<PlayerTag>()) {
            registry_.destroyEntity(player);
        }
    }

    // Handle save game request
    if (debugSession_.isSaveGameRequested) {
        debugSession_.isSaveGameRequested = false;
        logger::log(logger::DEBUG, "Saving game!");
        controller::PersistedGame persistedGame = getPersistedGame();
        controller::PersistenceManager::saveGame(persistedGame);
    }
}

void Game::updateSystems(const controller::InputState &input, float dt)
{
    controller::DebugContext &debug = controller::DebugContext::get();

    // Always update debug selection system
    debugSelectionSystem_.update(registry_, input, debug.active, debugSession_);

    if (debug.active && !debugSession_.isSystemUpdateActive) {
        return;
    }

    locationTable_.update(registry_);
    enemyAI_.update(registry_, locationTable_);
    inputSystem_.update(registry_, input);
    movementSystem_.update(registry_, dt);
    animationSystem_.update(registry_, dt);
    cameraSystem_.update(registry_);
}

bool Game::isWaveFinished()
{
    bool isWaveTimeFinished = currentWaveDuration_ >= config_.waveDurationSeconds;
    bool isWaveDefeated = registry_.view<EnemyTag>().empty();

    return isWaveDefeated | isWaveTimeFinished;
}

bool Game::isGameOver()
{
    return registry_.view<PlayerTag>().empty();
}

void Game::addScore(int score)
{
    score_ += score;
    currency_ += score;
}

void Game::updateView(view::View &view)
{
    // Game should be able to decide how to update.
    // Currently we simply clear everything and rebuilding the view from scratch.
    // Future improvements might only make changes and add/remove where necessary.
    view.nodes.clear();

    // Get camera data
    auto cameraEntities = registry_.view<CameraTag, Position>();
    if (!cameraEntities.empty()) {
        const Position &cameraPos = registry_.getComponent<Position>(cameraEntities.front());
        view.cameraX = cameraPos.x;
        view.cameraY = cameraPos.y;
    }

    // Render sprite entities
    for (auto entity : registry_.view<Position, view::Sprite>()) {

        view::Sprite &sprite = registry_.getComponent<view::Sprite>(entity);
        const Position &position = registry_.getComponent<Position>(entity);
        sprite.x = position.x;
        sprite.y = position.y;

        view.nodes.push_back({view::ViewMode::FixedToWorld, sprite});
    }

    stageWaveInfo_ = {
        .text = "Stage: " + std::to_string(stage_) + " Wave: " + std::to_string(wave_) + " Time remaining: "
                + std::to_string(config_.waveDurationSeconds - static_cast<int>(currentWaveDuration_))
                + " score: " + std::to_string(score_) + " currency: " + std::to_string(currency_),
        .size = 24,
        .gridX = 960.0f,
        .gridY = 75.0f,
    };
    view.nodes.push_back({view::ViewMode::FixedToScreen, std::cref(stageWaveInfo_)});
}
} // namespace game