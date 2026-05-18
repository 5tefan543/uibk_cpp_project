#include "game/ecs/components/camera.hpp"
#include "game/ecs/components/map.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/sprite.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/camera_system.hpp"
#include "shared/test_fixture.hpp"
#include "view/grid.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE_METHOD(TestFixture, "CameraSystem centers camera on center of player")
{
    game::Registry registry;
    game::CameraSystem system;

    game::Entity player = registry.createEntity();
    const game::Position playerPosition{2000.0f, 1200.0f};
    const game::Sprite playerSprite{
        .width = 128.0f,
        .height = 128.0f,
    };
    registry.addComponent<game::Position>(player, playerPosition);
    registry.addComponent<game::PlayerTag>(player, {});
    registry.addComponent<game::Sprite>(player, playerSprite);

    game::Entity world = registry.createEntity();
    registry.addComponent<game::Camera>(world, {0.0f, 0.0f, 128.0f});
    registry.addComponent<game::Map>(world, {.x = 0.0f, .y = 0.0f, .width = 3840.0f, .height = 2160.0f});

    system.update(registry);

    const auto &camera = registry.getComponent<game::Camera>(world);
    const float expectedCameraX = playerPosition.x + playerSprite.width / 2.0f - view::gridWidth / 2.0f;
    const float expectedCameraY = playerPosition.y + playerSprite.height / 2.0f - view::gridHeight / 2.0f;

    REQUIRE(camera.x == expectedCameraX);
    REQUIRE(camera.y == expectedCameraY);
}

TEST_CASE_METHOD(TestFixture, "CameraSystem clamps camera to minimum map boundary including margin")
{
    game::Registry registry;
    game::CameraSystem system;

    game::Entity player = registry.createEntity();
    const game::Position playerPosition{0.0f, 0.0f};
    const game::Sprite playerSprite{
        .width = 128.0f,
        .height = 128.0f,
    };
    registry.addComponent<game::Position>(player, playerPosition);
    registry.addComponent<game::PlayerTag>(player, {});
    registry.addComponent<game::Sprite>(player, playerSprite);

    game::Entity world = registry.createEntity();
    const game::Camera initialCamera{10.0f, 20.0f, 128.0f};
    const game::Map map{.x = 0.0f, .y = 0.0f, .width = 3840.0f, .height = 2160.0f};
    registry.addComponent<game::Camera>(world, initialCamera);
    registry.addComponent<game::Map>(world, map);

    system.update(registry);

    const auto &camera = registry.getComponent<game::Camera>(world);
    const float expectedCameraX = map.x - initialCamera.margin;
    const float expectedCameraY = map.y - initialCamera.margin;

    REQUIRE(camera.x == expectedCameraX);
    REQUIRE(camera.y == expectedCameraY);
}

TEST_CASE_METHOD(TestFixture, "CameraSystem clamps camera to maximum map boundary including margin")
{
    game::Registry registry;
    game::CameraSystem system;

    game::Entity player = registry.createEntity();
    const game::Position playerPosition{10000.0f, 8000.0f};
    const game::Sprite playerSprite{
        .width = 128.0f,
        .height = 128.0f,
    };
    registry.addComponent<game::Position>(player, playerPosition);
    registry.addComponent<game::PlayerTag>(player, {});
    registry.addComponent<game::Sprite>(player, playerSprite);

    game::Entity world = registry.createEntity();
    const game::Camera initialCamera{0.0f, 0.0f, 128.0f};
    const game::Map map{.x = 0.0f, .y = 0.0f, .width = 3840.0f, .height = 2160.0f};
    registry.addComponent<game::Camera>(world, initialCamera);
    registry.addComponent<game::Map>(world, map);

    system.update(registry);

    const auto &camera = registry.getComponent<game::Camera>(world);
    const float expectedCameraX = map.x + map.width - view::gridWidth + initialCamera.margin;
    const float expectedCameraY = map.y + map.height - view::gridHeight + initialCamera.margin;

    REQUIRE(camera.x == expectedCameraX);
    REQUIRE(camera.y == expectedCameraY);
}

TEST_CASE_METHOD(TestFixture, "CameraSystem does nothing if no player exists")
{
    game::Registry registry;
    game::CameraSystem system;

    game::Entity world = registry.createEntity();
    const game::Camera initialCamera{11.0f, 22.0f, 128.0f};
    const game::Map map{.x = 0.0f, .y = 0.0f, .width = 3840.0f, .height = 2160.0f};
    registry.addComponent<game::Camera>(world, initialCamera);
    registry.addComponent<game::Map>(world, map);

    system.update(registry);

    const auto &camera = registry.getComponent<game::Camera>(world);

    REQUIRE(camera.x == initialCamera.x);
    REQUIRE(camera.y == initialCamera.y);
}