#include "game/game.hpp"
#include "controller/debug/debug_context.hpp"
#include "controller/persistence/persisted_game.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/camera_tag.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/velocity.hpp"
#include "view/sprite.hpp"
#include <iostream>
#include <random>
#include <view/sprite.hpp>
#include <view/text.hpp>

namespace game {

Game::Game()
{
    std::cout << "Game constructed" << std::endl;
    config_ = controller::PersistenceManager::getConfig();
    initMap();
    initCamera();
}

Game::~Game()
{
    std::cout << "Game destructed" << std::endl;
}

void Game::initMap()
{
    // Initialize map and camera
    Entity map = registry_.createEntity();
    registry_.addComponent<MapTag>(map, {});
    registry_.addComponent<Position>(map, {0.0f, 0.0f});
    registry_.addComponent<view::Sprite>(map, {
                                                  .imagePath = "assets/maps/map.bmp",
                                                  .width = 1920.0f * 2.0f,
                                                  .height = 1080.0f * 2.0f,
                                              });
}

void Game::initCamera()
{
    Entity camera = registry_.createEntity();
    registry_.addComponent<CameraTag>(camera, {});
    registry_.addComponent<Position>(camera, {0.0f, 0.0f});
}

GameDebugSession &Game::getDebugSession()
{
    return debugSession_;
}

void Game::loadFromPersistedGame(const controller::PersistedGame &persistedGame)
{
    wave_ = persistedGame.wave;
    score_ = persistedGame.score;
    currency_ = persistedGame.currency;

    debugSession_.wave = wave_;

    auto players = registry_.view<PlayerTag>();
    if (!players.empty()) {
        PlayerTag &playerTag = registry_.getComponent<PlayerTag>(players.front());
        Position &position = registry_.getComponent<Position>(players.front());
        playerTag.moveSpeed = persistedGame.playerStats.speed;
        position.x = persistedGame.playerStats.posX;
        position.y = persistedGame.playerStats.posY;
    }
}

controller::PersistedGame Game::getPersistedGame() const
{
    controller::PersistedGame persistedGame;
    persistedGame.wave = wave_;
    persistedGame.score = score_;
    persistedGame.currency = currency_;

    auto players = registry_.view<PlayerTag>();
    if (!players.empty()) {
        const PlayerTag &playerTag = registry_.getComponent<PlayerTag>(players.front());
        const Position &position = registry_.getComponent<Position>(players.front());
        persistedGame.playerStats.speed = playerTag.moveSpeed;
        persistedGame.playerStats.posX = position.x;
        persistedGame.playerStats.posY = position.y;
    }

    return persistedGame;
}

controller::StateTransitionAction Game::update(const controller::InputState &input, float dt)
{
    initialize();
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

void Game::initialize()
{
    if (!isInitialized_) {
        isInitialized_ = true;
        initPlayer();
        initWave(wave_);
    }
}

void Game::initPlayer()
{
    Entity player = registry_.createEntity();
    registry_.addComponent<PlayerTag>(player, {});
    registry_.addComponent<Position>(player, {100.0f, 100.0f});
    registry_.addComponent<Velocity>(player, {0.0f, 0.0f});
    registry_.addComponent<view::Sprite>(player, {});
    registry_.addComponent<Animation>(player, {
                                                  .baseTexturePath = "assets/characters/character_",
                                              });
}

void Game::initWave(int waveNumber)
{
    // delete all existing enemies
    for (Entity enemy : registry_.view<EnemyTag>()) {
        registry_.destroyEntity(enemy);
    }

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

    // spawn enemies for the new wave
    initEnemies();

    std::cout << "Starting wave " << wave_ << " of stage " << stage_ << std::endl;
}

void Game::initEnemies()
{
    // TODO Real spawning logic based on wavecount here.
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> posDist(200.0f, 800.0f);
    std::uniform_real_distribution<> velDist(0.0f, 10.0f);

    // Spawn 3 enemies at different positions
    for (int i = 0; i < 3; ++i) {
        Entity enemy = registry_.createEntity();
        registry_.addComponent<EnemyTag>(enemy, {});
        registry_.addComponent<Position>(enemy, {static_cast<float>(posDist(gen)), static_cast<float>(posDist(gen))});
        registry_.addComponent<Velocity>(enemy, {static_cast<float>(velDist(gen)), static_cast<float>(velDist(gen))});
        registry_.addComponent<view::Sprite>(enemy, {});
        registry_.addComponent<Animation>(enemy, {
                                                     .baseTexturePath = "assets/characters/enemy_1_",
                                                 });
    }
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
        std::cout << "Destroying player entity!" << std::endl;
        for (Entity player : registry_.view<PlayerTag>()) {
            registry_.destroyEntity(player);
        }
    }

    // Handle save game request
    if (debugSession_.isSaveGameRequested) {
        debugSession_.isSaveGameRequested = false;
        std::cout << "Saving game!" << std::endl;
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