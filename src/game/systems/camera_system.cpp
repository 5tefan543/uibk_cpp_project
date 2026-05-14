#include "game/ecs/systems/camera_system.hpp"
#include "game/ecs/components/camera.hpp"
#include "game/ecs/components/map.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/sprite.hpp"
#include "view/grid.hpp"

#include <algorithm>

namespace game {

void CameraSystem::update(Registry &registry)
{
    // Find player and camera
    auto players = registry.view<Position, PlayerTag, Sprite>();
    auto cameras = registry.view<Camera, Map>();

    if (players.empty() || cameras.empty()) {
        return;
    }

    const Entity playerEntity = players.front();
    const Entity cameraEntity = cameras.front();

    const Position &playerPosition = registry.getComponent<Position>(playerEntity);
    const Sprite &playerSprite = registry.getComponent<Sprite>(playerEntity);

    Camera &camera = registry.getComponent<Camera>(cameraEntity);
    const Map &map = registry.getComponent<Map>(cameraEntity);

    const float playerCenterX = playerPosition.x + playerSprite.width / 2.0f;
    const float playerCenterY = playerPosition.y + playerSprite.height / 2.0f;

    const float desiredCameraX = playerCenterX - view::gridWidth / 2.0f;
    const float desiredCameraY = playerCenterY - view::gridHeight / 2.0f;

    const float minCameraX = map.x - camera.margin;
    const float minCameraY = map.y - camera.margin;

    const float maxCameraX = map.x + map.width - view::gridWidth + camera.margin;
    const float maxCameraY = map.y + map.height - view::gridHeight + camera.margin;

    camera.x = std::clamp(desiredCameraX, minCameraX, maxCameraX);
    camera.y = std::clamp(desiredCameraY, minCameraY, maxCameraY);
}

} // namespace game
