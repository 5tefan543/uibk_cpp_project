#include "game/ecs/components/camera_tag.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/camera_system.hpp"
#include "shared/test_fixture.hpp"
#include "view/grid.hpp"
#include "view/sprite.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE_METHOD(TestFixture, "CameraSystem centers camera on center of player")
{
    game::Registry registry;
    game::CameraSystem system;

    game::Entity player = registry.createEntity();
    const game::Position playerPosition{2000.0f, 1200.0f};
    const view::Sprite playerSprite{.rect = {.position = {0, 0}, .size = {128.0f, 128.0f}}};
    registry.addComponent<game::Position>(player, playerPosition);
    registry.addComponent<game::PlayerTag>(player, {});
    registry.addComponent<view::Sprite>(player, playerSprite);

    game::Entity camera = registry.createEntity();
    registry.addComponent<game::Position>(camera, {0.0f, 0.0f});
    registry.addComponent<game::CameraTag>(camera, {128.0f});

    game::Entity map = registry.createEntity();
    registry.addComponent<game::Position>(map, {0.0f, 0.0f});
    registry.addComponent<view::Sprite>(map, {.rect = {{0, 0}, {3840.0f, 2160.0f}}});
    registry.addComponent<game::MapTag>(map, {});

    system.update(registry);

    const auto &cameraPos = registry.getComponent<game::Position>(camera).p;
    const geometry::Vec2<float> expectedCameraPos =
        playerPosition.p + playerSprite.rect.size / 2.0f - view::grid.size / 2.0f;

    REQUIRE((cameraPos == expectedCameraPos).all());
}

TEST_CASE_METHOD(TestFixture, "CameraSystem clamps camera to minimum map boundary including margin")
{
    game::Registry registry;
    game::CameraSystem system;

    game::Entity player = registry.createEntity();
    const game::Position playerPosition{0.0f, 0.0f};
    const view::Sprite playerSprite{.rect = {.position = {0, 0}, .size = {128.0f, 128.0f}}};
    registry.addComponent<game::Position>(player, playerPosition);
    registry.addComponent<game::PlayerTag>(player, {});
    registry.addComponent<view::Sprite>(player, playerSprite);

    game::Entity camera = registry.createEntity();
    const game::CameraTag cameraTag{128.0f};
    registry.addComponent<game::Position>(camera, {10.0f, 20.0f});
    registry.addComponent<game::CameraTag>(camera, cameraTag);

    game::Entity map = registry.createEntity();
    const game::Position mapPos{0.0f, 0.0f};
    const view::Sprite mapSprite{.rect = {{0, 0}, {3840.0f, 2160.0f}}};
    registry.addComponent<game::Position>(map, mapPos);
    registry.addComponent<view::Sprite>(map, mapSprite);
    registry.addComponent<game::MapTag>(map, {});

    system.update(registry);

    const auto &cameraPos = registry.getComponent<game::Position>(camera).p;
    const geometry::Vec2<float> expectedCameraPos = mapPos.p - cameraTag.margin;

    REQUIRE((cameraPos == expectedCameraPos).all());
}

TEST_CASE_METHOD(TestFixture, "CameraSystem clamps camera to maximum map boundary including margin")
{
    game::Registry registry;
    game::CameraSystem system;

    game::Entity player = registry.createEntity();
    const game::Position playerPosition{10000.0f, 8000.0f};
    const view::Sprite playerSprite{.rect = {.position = {0, 0}, .size = {128.0f, 128.0f}}};
    registry.addComponent<game::Position>(player, playerPosition);
    registry.addComponent<game::PlayerTag>(player, {});
    registry.addComponent<view::Sprite>(player, playerSprite);

    game::Entity camera = registry.createEntity();
    const game::CameraTag cameraTag{128.0f};
    registry.addComponent<game::Position>(camera, {10.0f, 20.0f});
    registry.addComponent<game::CameraTag>(camera, cameraTag);

    game::Entity map = registry.createEntity();
    const game::Position mapPos{0.0f, 0.0f};
    const view::Sprite mapSprite{.rect = {{0, 0}, {3840.0f, 2160.0f}}};
    registry.addComponent<game::Position>(map, mapPos);
    registry.addComponent<view::Sprite>(map, mapSprite);
    registry.addComponent<game::MapTag>(map, {});

    system.update(registry);

    const auto &cameraPos = registry.getComponent<game::Position>(camera).p;
    const geometry::Vec2<float> expectedCameraPos = mapPos.p + mapSprite.rect.size - view::grid.size + cameraTag.margin;

    REQUIRE((cameraPos == expectedCameraPos).all());
}

TEST_CASE_METHOD(TestFixture, "CameraSystem does nothing if no player exists")
{
    game::Registry registry;
    game::CameraSystem system;

    game::Entity camera = registry.createEntity();
    const game::Position initialCameraPos{11.0f, 22.0f};
    const game::CameraTag cameraTag{128.0f};
    registry.addComponent<game::Position>(camera, initialCameraPos);
    registry.addComponent<game::CameraTag>(camera, cameraTag);

    game::Entity map = registry.createEntity();
    const view::Sprite mapSprite{.rect = {{0, 0}, {3840.0f, 2160.0f}}};
    registry.addComponent<game::Position>(map, {0.0f, 0.0f});
    registry.addComponent<view::Sprite>(map, mapSprite);
    registry.addComponent<game::MapTag>(map, {});

    system.update(registry);

    const auto &cameraPos = registry.getComponent<game::Position>(camera).p;

    REQUIRE((cameraPos == initialCameraPos.p).all());
}