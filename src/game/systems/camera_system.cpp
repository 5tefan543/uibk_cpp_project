#include "game/ecs/systems/camera_system.hpp"
#include "game/ecs/components/camera_tag.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "view/grid.hpp"
#include "view/sprite.hpp"

#include <algorithm>

namespace game {

void CameraSystem::update(Registry &registry)
{
    // Find player and camera
    auto players = registry.view<Position, PlayerTag, view::Sprite>();
    auto maps = registry.view<MapTag, Position, view::Sprite>();
    auto cameras = registry.view<CameraTag, Position>();

    if (players.empty() || maps.empty() || cameras.empty()) {
        return;
    }

    const Entity playerEntity = players.front();
    const Entity mapEntity = maps.front();
    const Entity cameraEntity = cameras.front();

    const Position &playerPosition = registry.getComponent<Position>(playerEntity);
    const view::Sprite &playerSprite = registry.getComponent<view::Sprite>(playerEntity);

    const Position &mapPos = registry.getComponent<Position>(mapEntity);
    const view::Sprite &mapSprite = registry.getComponent<view::Sprite>(mapEntity);

    const CameraTag &cameraTag = registry.getComponent<CameraTag>(cameraEntity);
    Position &cameraPos = registry.getComponent<Position>(cameraEntity);

    const float playerCenterX = playerPosition.x + playerSprite.width / 2.0f;
    const float playerCenterY = playerPosition.y + playerSprite.height / 2.0f;

    const float desiredCameraX = playerCenterX - view::gridWidth / 2.0f;
    const float desiredCameraY = playerCenterY - view::gridHeight / 2.0f;

    const float minCameraX = mapPos.x - cameraTag.margin;
    const float minCameraY = mapPos.y - cameraTag.margin;

    const float maxCameraX = mapPos.x + mapSprite.width - view::gridWidth + cameraTag.margin;
    const float maxCameraY = mapPos.y + mapSprite.height - view::gridHeight + cameraTag.margin;

    cameraPos.x = std::clamp(desiredCameraX, minCameraX, maxCameraX);
    cameraPos.y = std::clamp(desiredCameraY, minCameraY, maxCameraY);
}

} // namespace game
