#include "game/game.hpp"
#include "game/ecs/components/camera.hpp"
#include "game/ecs/components/map.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/sprite.hpp"
#include "game/ecs/components/velocity.hpp"
#include <controller/view/sprite.hpp>
#include <controller/view/text.hpp>
#include <iostream>

namespace game {

Game::Game()
{
    std::cout << "Game constructed" << std::endl;
    initPlayer();
}

Game::~Game()
{
    std::cout << "Game destructed" << std::endl;
}

void Game::initPlayer()
{
    Entity player = registry.createEntity();
    registry.addComponent<PlayerTag>(player, {});
    registry.addComponent<Position>(player, {100.0f, 100.0f});
    registry.addComponent<Velocity>(player, {0.0f, 0.0f});
    registry.addComponent<Sprite>(player, {});

    // Initialize map and camera
    Entity mapEntity = registry.createEntity();
    registry.addComponent<Map>(mapEntity, {});
    registry.addComponent<Camera>(mapEntity, {});
}

controller::StateTransitionAction Game::update(const controller::InputState &input, float dt)
{
    inputSystem.update(registry, input);
    movementSystem.update(registry, dt);
    animationSystem.update(registry, dt);
    cameraSystem.update(registry);

    if (input.cancelPressed) {
        return controller::StateTransitionAction::PushPauseMenu;
    } else if (input.mouseLeftPressed) {
        return controller::StateTransitionAction::PushProgressionStore;
    } else if (input.confirmPressed) {
        return controller::StateTransitionAction::ReplaceCurrentWithGameOverMenu;
    }
    return controller::StateTransitionAction::None;
}

controller::View Game::getView()
{
    controller::View view;

    // Get camera data
    auto cameraEntities = registry.view<Camera, Map>();
    if (!cameraEntities.empty()) {
        const Camera &camera = registry.getComponent<Camera>(cameraEntities[0]);
        const Map &map = registry.getComponent<Map>(cameraEntities[0]);
        view.cameraX = camera.x;
        view.cameraY = camera.y;
        view.mapWidth = map.width;
        view.mapHeight = map.height;

        // Add map sprite
        controller::Sprite mapSprite;
        mapSprite.x = map.x;
        mapSprite.y = map.y;
        mapSprite.imagePath = map.texturePath;
        mapSprite.width = map.width;
        mapSprite.height = map.height;
        mapSprite.scale = 2.0f; // Map scaled by 2x
        mapSprite.isMap = true; // Mark as map so renderer doesn't apply camera offset
        view.items.push_back(mapSprite);
    }

    // Render sprite entities
    for (auto entity : registry.view<Position, Sprite>()) {
        const Position &position = registry.getComponent<Position>(entity);
        const Sprite &gameSprite = registry.getComponent<Sprite>(entity);

        // Build texture path based on direction and frame
        std::string directionStr = (gameSprite.direction == Direction::Left) ? "left" : "right";
        int frameNum = gameSprite.currentFrame + 1; // Frames are 1-indexed in filenames
        std::string imagePath =
            gameSprite.baseTexturePath + "character_" + directionStr + "_" + std::to_string(frameNum) + ".bmp";

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