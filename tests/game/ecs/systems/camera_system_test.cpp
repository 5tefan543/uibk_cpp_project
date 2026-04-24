#include "game/ecs/components/camera.hpp"
#include "game/ecs/components/map.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/camera_system.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("CameraSystem centers camera on player")
{
    // ARRANGE
    game::Registry registry;
    game::CameraSystem system;

    game::Entity player = registry.createEntity();
    registry.addComponent<game::Position>(player, {500.0f, 300.0f});
    registry.addComponent<game::PlayerTag>(player, {});

    game::Entity world = registry.createEntity();
    registry.addComponent<game::Camera>(world, {0.0f, 0.0f, 960.0f, 540.0f});
    registry.addComponent<game::Map>(world, {0.0f, 0.0f, 5000.0f, 3000.0f, "assets/maps/map.bmp"});

    // ACT
    system.update(registry);

    // ASSERT
    const auto &camera = registry.getComponent<game::Camera>(world);
    REQUIRE(camera.x == (500.0f + 64.0f - 960.0f / 2.0f));
    REQUIRE(camera.y == (300.0f + 64.0f - 540.0f / 2.0f));
}

TEST_CASE("CameraSystem clamps camera to minimum boundaries")
{
    // ARRANGE
    game::Registry registry;
    game::CameraSystem system;

    game::Entity player = registry.createEntity();
    registry.addComponent<game::Position>(player, {0.0f, 0.0f});
    registry.addComponent<game::PlayerTag>(player, {});

    game::Entity world = registry.createEntity();
    registry.addComponent<game::Camera>(world, {10.0f, 20.0f, 960.0f, 540.0f});
    registry.addComponent<game::Map>(world, {0.0f, 0.0f, 1920.0f, 1080.0f, "assets/maps/map.bmp"});

    // ACT
    system.update(registry);

    // ASSERT
    const auto &camera = registry.getComponent<game::Camera>(world);
    REQUIRE(camera.x == -64.0f);
    REQUIRE(camera.y == -64.0f);
}

TEST_CASE("CameraSystem clamps camera to maximum map boundaries")
{
    // ARRANGE
    game::Registry registry;
    game::CameraSystem system;

    game::Entity player = registry.createEntity();
    registry.addComponent<game::Position>(player, {10000.0f, 8000.0f});
    registry.addComponent<game::PlayerTag>(player, {});

    game::Entity world = registry.createEntity();
    registry.addComponent<game::Camera>(world, {0.0f, 0.0f, 960.0f, 540.0f});
    registry.addComponent<game::Map>(world, {0.0f, 0.0f, 1920.0f, 1080.0f, "assets/maps/map.bmp"});

    // ACT
    system.update(registry);

    // ASSERT
    const auto &camera = registry.getComponent<game::Camera>(world);
    REQUIRE(camera.x == 1920.0f);
    REQUIRE(camera.y == 1080.0f);
}

TEST_CASE("CameraSystem does nothing if no player exists")
{
    // ARRANGE
    game::Registry registry;
    game::CameraSystem system;

    game::Entity world = registry.createEntity();
    registry.addComponent<game::Camera>(world, {11.0f, 22.0f, 960.0f, 540.0f});
    registry.addComponent<game::Map>(world, {0.0f, 0.0f, 1920.0f, 1080.0f, "assets/maps/map.bmp"});

    // ACT
    system.update(registry);

    // ASSERT
    const auto &camera = registry.getComponent<game::Camera>(world);
    REQUIRE(camera.x == 11.0f);
    REQUIRE(camera.y == 22.0f);
}
