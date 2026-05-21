#include "game/game.hpp"
#include "controller/debug/debug_context.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "game/ecs/components/camera.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/map.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/sprite.hpp"
#include "game/ecs/components/velocity.hpp"
#include <iostream>
#include <random>
#include <view/sprite.hpp>
#include <view/text.hpp>
namespace game {

Game::Game() : locationTable_(40, 40) // 1920/40 = 48 buckets; 1080/40 = 27 buckets
{
#ifdef LOG_STDOUT
    std::cout << "Game constructed" << std::endl;
#endif
    initStage();
    initWave();
    initPlayer();
    initEnemies();
}

void Game::initStage()
{
    // Initialize map and camera
    Entity mapEntity = registry_.createEntity();
    registry_.addComponent<Map>(mapEntity, {});
    registry_.addComponent<Camera>(mapEntity, {});
}

void Game::initWave()
{
#ifdef LOG_STDOUT
    std::cout << "Loading stage " << stage_ << ", wave " << wave_ << std::endl;
#endif
    // Logic to initialize a new wave of enemies can go here
}

void Game::initPlayer()
{
    Entity player = registry_.createEntity();
    registry_.addComponent<PlayerTag>(player, {});
    registry_.addComponent<Position>(player, {100.0f, 100.0f});
    registry_.addComponent<Velocity>(player, {0.0f, 0.0f});
    registry_.addComponent<Sprite>(player, {});
}

void Game::initEnemies()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> posDist(200.0f, 800.0f);
    std::uniform_real_distribution<> velDist(0.0f, 10.0f);

    // Spawn 3 enemies at different positions
    for (int i = 0; i < 50; ++i) {
        Entity enemy = registry_.createEntity();
        registry_.addComponent<EnemyTag>(enemy, {});
        registry_.addComponent<Position>(enemy, {static_cast<float>(posDist(gen)), static_cast<float>(posDist(gen))});
        registry_.addComponent<Velocity>(enemy, {static_cast<float>(velDist(gen)), static_cast<float>(velDist(gen))});

        // Set sprite with enemy texture
        Sprite sprite;
        sprite.baseTexturePath = "assets/characters/enemy_1_";
        registry_.addComponent<Sprite>(enemy, sprite);
    }
}

Game::~Game()
{
#ifdef LOG_STDOUT
    std::cout << "Game destructed" << std::endl;
#endif
}

GameDebugSession &Game::getDebugSession()
{
    return debugSession_;
}

void Game::loadFromPersistedGame(const controller::PersistedGame &persistedGame)
{
    stage_ = persistedGame.stage;
    wave_ = persistedGame.wave;
    currency_ = persistedGame.currency;

    // Update debug session with loaded stage/wave
    debugSession_.stage = stage_;
    debugSession_.wave = wave_;

    auto players = registry_.view<PlayerTag>();
    if (!players.empty()) {
        PlayerTag &playerTag = registry_.getComponent<PlayerTag>(players.front());
        playerTag.moveSpeed = persistedGame.playerStats.speed;
    }

    // currently when loading a game, we init the wave 2 times
    // once in Game constructor and once here
    // This is because the game is created already in GameplayState
    initWave();
}

controller::PersistedGame Game::getPersistedGame() const
{
    controller::PersistedGame persistedGame;
    persistedGame.stage = stage_;
    persistedGame.wave = wave_;
    persistedGame.currency = currency_;

    auto players = registry_.view<PlayerTag>();
    if (!players.empty()) {
        const PlayerTag &playerTag = registry_.getComponent<PlayerTag>(players.front());
        persistedGame.playerStats.speed = playerTag.moveSpeed;
    }

    return persistedGame;
}

bool Game::update(const controller::InputState &input, float dt)
{
    processDebugSession();
    updateSystems(input, dt);
    return isGameOver();
}

void Game::processDebugSession()
{
    controller::DebugContext &debug = controller::DebugContext::get();

    if (!debug.active) {
        return;
    }

    // Handle stage/wave reload request
    if (debugSession_.isStageWaveReloadRequested) {
        debugSession_.isStageWaveReloadRequested = false;
        stage_ = debugSession_.stage;
        wave_ = debugSession_.wave;
        initWave();
    }

    // Handle player destruction request
    if (debugSession_.isPlayerDestructionRequested) {
        debugSession_.isPlayerDestructionRequested = false;
#ifdef LOG_STDOUT
        std::cout << "Destroying player entity!" << std::endl;
#endif
        for (Entity player : registry_.view<PlayerTag>()) {
            registry_.destroyEntity(player);
        }
    }

    // Handle save game request
    if (debugSession_.isSaveGameRequested) {
        debugSession_.isSaveGameRequested = false;
#ifdef LOG_STDOUT
        std::cout << "Saving game!" << std::endl;
#endif
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

bool Game::isGameOver()
{
    return registry_.view<PlayerTag>().empty();
}

void Game::updateView(view::View &view)
{
    // Game should be able to decide how to update.
    // Currently we simply clear everything and rebuilding the view from scratch.
    // Future improvements might only make changes and add/remove where necessary.
    view.nodes.clear();

    // Get camera data
    auto cameraEntities = registry_.view<Camera, Map>();
    if (!cameraEntities.empty()) {
        const Camera &camera = registry_.getComponent<Camera>(cameraEntities.front());
        const Map &map = registry_.getComponent<Map>(cameraEntities.front());
        view.cameraX = camera.x;
        view.cameraY = camera.y;

        // Add map sprite
        view::Sprite mapSprite;
        mapSprite.x = map.x;
        mapSprite.y = map.y;
        mapSprite.imagePath = map.texturePath;
        mapSprite.width = map.width;
        mapSprite.height = map.height;
        mapSprite.isSelected = map.isSelected;
        view.nodes.push_back({view::ViewMode::FixedToWorld, mapSprite});
    }

    // Render sprite entities
    for (auto entity : registry_.view<Position, Sprite>()) {
        const Position &position = registry_.getComponent<Position>(entity);
        const Sprite &gameSprite = registry_.getComponent<Sprite>(entity);

        // Build texture path based on direction and frame
        std::string directionStr = (gameSprite.direction == Direction::Left) ? "left" : "right";
        int frameNum = gameSprite.currentFrame + 1; // Frames are 1-indexed in filenames

        std::string imagePath;
        if (gameSprite.baseTexturePath.find("enemy") != std::string::npos) {
            // Enemy sprite path doesn't need "character_" prefix
            imagePath = gameSprite.baseTexturePath + directionStr + "_" + std::to_string(frameNum) + ".png";
        } else {
            // Player sprite path needs "character_" prefix
            imagePath =
                gameSprite.baseTexturePath + "character_" + directionStr + "_" + std::to_string(frameNum) + ".png";
        }

        view::Sprite viewSprite;
        viewSprite.x = position.x;
        viewSprite.y = position.y;
        viewSprite.imagePath = imagePath;
        viewSprite.width = gameSprite.width;
        viewSprite.height = gameSprite.height;
        viewSprite.isSelected = gameSprite.isSelected;
        view.nodes.push_back({view::ViewMode::FixedToWorld, viewSprite});
    }

    stageWaveInfo_ = {
        .text = "Stage: " + std::to_string(stage_) + " Wave: " + std::to_string(wave_),
        .size = 24,
        .gridX = 960.0f,
        .gridY = 75.0f,
    };
    view.nodes.push_back({view::ViewMode::FixedToScreen, std::cref(stageWaveInfo_)});
}
} // namespace game