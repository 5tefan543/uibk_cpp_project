#include "game/game.hpp"
#include "game/ecs/components/camera.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/map.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/sprite.hpp"
#include "game/ecs/components/velocity.hpp"
#include <controller/view/sprite.hpp>
#include <controller/view/text.hpp>
#include <iostream>
#include <random>

namespace game {

Game::Game()
{
    std::cout << "Game constructed" << std::endl;
    initStage();
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
    // Logic to initialize a new wave of enemies can go here
}

void Game::initPlayer()
{
    Entity player = registry_.createEntity();
    registry_.addComponent<PlayerTag>(player, {});
    registry_.addComponent<Position>(player, {100.0f, 100.0f});
    registry_.addComponent<Velocity>(player, {0.0f, 0.0f});
    registry_.addComponent<Sprite>(player, {});

    // Initialize map and camera
    Entity mapEntity = registry_.createEntity();
    registry_.addComponent<Map>(mapEntity, {});
    registry_.addComponent<Camera>(mapEntity, {});
}

void Game::initEnemies()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> posDist(200.0f, 800.0f);
    std::uniform_real_distribution<> velDist(0.0f, 0.0f);

    // Spawn 3 enemies at different positions
    for (int i = 0; i < 3; ++i) {
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
    std::cout << "Game destructed" << std::endl;
}

GameDebugSession &Game::getDebugSession()
{
    return debugSession_;
}

bool Game::update(const controller::InputState &input, controller::DebugContext &debug, float dt)
{
    processDebugSession(debug);
    updateSystems(input, debug, dt);
    return isGameOver();
}

void Game::processDebugSession(controller::DebugContext &debug)
{
    if (!debug.active) {
        return;
    }

    // Handle stage/wave reload request
    if (debugSession_.isStageWaveReloadRequested) {
        debugSession_.isStageWaveReloadRequested = false;
        std::cout << "Reloading stage " << debug.gameSettings.stage << ", wave " << debug.gameSettings.wave
                  << std::endl;
        // TODO: Implement actual stage/wave reloading logic
    }

    // Handle player destruction request
    if (debugSession_.isPlayerDestructionRequested) {
        debugSession_.isPlayerDestructionRequested = false;
        std::cout << "Destroying player entity!" << std::endl;
        for (Entity player : registry_.view<PlayerTag>()) {
            registry_.destroyEntity(player);
        }
    }
}

void Game::updateSystems(const controller::InputState &input, controller::DebugContext &debug, float dt)
{
    if (debug.active && !debugSession_.isSystemUpdateActive) {
        return;
    }

    inputSystem_.update(registry_, input);
    movementSystem_.update(registry_, dt);
    animationSystem_.update(registry_, dt);
    cameraSystem_.update(registry_);
}

bool Game::isGameOver()
{
    return registry_.view<PlayerTag>().empty();
}

controller::View Game::getView()
{
    controller::View view;

    // Get camera data
    auto cameraEntities = registry_.view<Camera, Map>();
    if (!cameraEntities.empty()) {
        const Camera &camera = registry_.getComponent<Camera>(cameraEntities.front());
        const Map &map = registry_.getComponent<Map>(cameraEntities.front());
        view.cameraX = camera.x;
        view.cameraY = camera.y;
        view.mapWidth = map.width;
        view.mapHeight = map.height;

        // Add map sprite
        controller::Sprite mapSprite;
        mapSprite.x = map.x - camera.x;
        mapSprite.y = map.y - camera.y;
        mapSprite.imagePath = map.texturePath;
        mapSprite.width = map.width;
        mapSprite.height = map.height;
        mapSprite.scale = 2.0f; // Map scaled by 2x
        view.items.push_back(mapSprite);
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

        controller::Sprite viewSprite;
        viewSprite.x = position.x;
        viewSprite.y = position.y;
        viewSprite.imagePath = imagePath;
        viewSprite.width = 32.0f;
        viewSprite.height = 32.0f;
        viewSprite.scale = 4.0f; // Character scaled by 4x
        view.items.push_back(viewSprite);
    }

    return view;
}
} // namespace game