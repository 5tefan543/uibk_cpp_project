#include "game/ecs/systems/camera_system.hpp"
#include "game/ecs/components/camera.hpp"
#include "game/ecs/components/map.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"

#include <algorithm>

namespace game {

void CameraSystem::update(Registry &registry)
{
    // Find player and camera
    auto players = registry.view<Position, PlayerTag>();
    auto cameras = registry.view<Camera, Map>();

    if (players.empty() || cameras.empty()) {
        return;
    }

    const Position &playerPos = registry.getComponent<Position>(players[0]);
    Camera &camera = registry.getComponent<Camera>(cameras[0]);
    const Map &map = registry.getComponent<Map>(cameras[0]);

    // Center camera on player (considering character is scaled by 4, so 128x128 pixels)
    float playerCenterX = playerPos.x + 64.0f; // 32 * 4 / 2
    float playerCenterY = playerPos.y + 64.0f;

    // Set camera to center on player
    camera.x = playerCenterX - camera.viewportWidth / 2.0f;
    camera.y = playerCenterY - camera.viewportHeight / 2.0f;

    // Clamp camera to map boundaries
    // camera.x = std::max(0.0f, std::min(camera.x, map.width));
    // camera.y = std::max(0.0f, std::min(camera.y, map.height));
}

} // namespace game
