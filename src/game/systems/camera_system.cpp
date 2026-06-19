#include "game/ecs/systems/camera_system.hpp"
#include "game/ecs/components/camera_tag.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "view/grid.hpp"
#include "view/sprite.hpp"

namespace game {

void CameraSystem::update(Registry &registry)
{
    using geometry::Vec2;

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

    const auto &playerPosition = registry.getComponent<Position>(playerEntity).p;
    const view::Sprite &playerSprite = registry.getComponent<view::Sprite>(playerEntity);

    const auto &mapPos = registry.getComponent<Position>(mapEntity).p;
    const view::Sprite &mapSprite = registry.getComponent<view::Sprite>(mapEntity);

    const CameraTag &cameraTag = registry.getComponent<CameraTag>(cameraEntity);
    auto &cameraPos = registry.getComponent<Position>(cameraEntity).p;

    const auto playerCenter = geometry::Rectangle{playerPosition, playerSprite.rect.size}.getCenter();
    const auto desiredCamera = playerCenter - view::grid.getCenter();
    const auto minCamera = mapPos - cameraTag.margin;
    const auto maxCamera = mapPos + mapSprite.rect.size - view::grid.size + cameraTag.margin;
    cameraPos = desiredCamera.clamp(minCamera, maxCamera);
}

} // namespace game
