#include "game/debug/game_debug_session.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/debug_selection_system.hpp"
#include "shared/test_fixture.hpp"
#include "view/sprite.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE_METHOD(TestFixture, "DebugSelectionSystem selects sprite under mouse when ctrl-clicked")
{
    game::LocationTable unused({0, 0}, {0, 0});
    game::Registry registry;
    game::DebugSelectionSystem system;
    game::GameDebugSession debugSession(registry, unused);

    game::Entity spriteEntity = registry.createEntity();
    game::Position spritePosition{.x = 100.0f, .y = 100.0f};
    view::Sprite spriteComponent{.width = 32.0f, .height = 32.0f};

    registry.addComponent(spriteEntity, spritePosition);
    registry.addComponent(spriteEntity, spriteComponent);

    controller::InputState input{};
    input.controlHeld = true;
    input.mouseLeftPressed = true;
    input.mouseGridX = spritePosition.x;
    input.mouseGridY = spritePosition.y;

    system.update(registry, input, true, debugSession);

    REQUIRE(debugSession.selectedEntity == spriteEntity);
    REQUIRE(registry.getComponent<view::Sprite>(spriteEntity).isSelected);
}

TEST_CASE_METHOD(TestFixture, "DebugSelectionSystem selects map when only map is under mouse")
{
    game::LocationTable unused({0, 0}, {0, 0});
    game::Registry registry;
    game::DebugSelectionSystem system;
    game::GameDebugSession debugSession(registry, unused);

    game::Entity mapEntity = registry.createEntity();
    game::Position mapPosition{.x = 100.0f, .y = 100.0f};
    view::Sprite mapSprite{.width = 500.0f, .height = 500.0f};

    registry.addComponent(mapEntity, mapPosition);
    registry.addComponent(mapEntity, mapSprite);
    registry.addComponent(mapEntity, game::MapTag{});

    controller::InputState input{};
    input.controlHeld = true;
    input.mouseLeftPressed = true;
    input.mouseGridX = mapPosition.x;
    input.mouseGridY = mapPosition.y;

    system.update(registry, input, true, debugSession);

    REQUIRE(debugSession.selectedEntity == mapEntity);
    REQUIRE(registry.getComponent<view::Sprite>(mapEntity).isSelected);
}

TEST_CASE_METHOD(TestFixture, "DebugSelectionSystem prefers non-map sprite over map when both are under mouse")
{
    game::LocationTable unused({0, 0}, {0, 0});
    game::Registry registry;
    game::DebugSelectionSystem system;
    game::GameDebugSession debugSession(registry, unused);

    game::Entity mapEntity = registry.createEntity();
    registry.addComponent(mapEntity, game::Position{.x = 0.0f, .y = 0.0f});
    registry.addComponent(mapEntity, view::Sprite{.width = 500.0f, .height = 500.0f});
    registry.addComponent(mapEntity, game::MapTag{});

    game::Entity spriteEntity = registry.createEntity();
    registry.addComponent(spriteEntity, game::Position{.x = 100.0f, .y = 100.0f});
    registry.addComponent(spriteEntity, view::Sprite{.width = 32.0f, .height = 32.0f});

    controller::InputState input{};
    input.controlHeld = true;
    input.mouseLeftPressed = true;
    input.mouseGridX = 100.0f;
    input.mouseGridY = 100.0f;

    system.update(registry, input, true, debugSession);

    REQUIRE(debugSession.selectedEntity == spriteEntity);
    REQUIRE(registry.getComponent<view::Sprite>(spriteEntity).isSelected);
    REQUIRE_FALSE(registry.getComponent<view::Sprite>(mapEntity).isSelected);
}

TEST_CASE_METHOD(TestFixture, "DebugSelectionSystem does not select entity when ctrl is not held")
{
    game::LocationTable unused({0, 0}, {0, 0});
    game::Registry registry;
    game::DebugSelectionSystem system;
    game::GameDebugSession debugSession(registry, unused);

    game::Entity spriteEntity = registry.createEntity();
    game::Position spritePosition{.x = 100.0f, .y = 100.0f};
    view::Sprite spriteComponent{.width = 32.0f, .height = 32.0f};

    registry.addComponent(spriteEntity, spritePosition);
    registry.addComponent(spriteEntity, spriteComponent);

    controller::InputState input{};
    input.controlHeld = false;
    input.mouseLeftPressed = true;
    input.mouseGridX = spritePosition.x;
    input.mouseGridY = spritePosition.y;

    system.update(registry, input, true, debugSession);

    REQUIRE_FALSE(debugSession.selectedEntity.has_value());
    REQUIRE_FALSE(registry.getComponent<view::Sprite>(spriteEntity).isSelected);
}

TEST_CASE_METHOD(TestFixture, "DebugSelectionSystem does not select entity when mouse is outside entity")
{
    game::LocationTable unused({0, 0}, {0, 0});
    game::Registry registry;
    game::DebugSelectionSystem system;
    game::GameDebugSession debugSession(registry, unused);

    game::Entity spriteEntity = registry.createEntity();
    game::Position spritePosition{.x = 100.0f, .y = 100.0f};
    view::Sprite spriteComponent{.width = 32.0f, .height = 32.0f};

    registry.addComponent(spriteEntity, spritePosition);
    registry.addComponent(spriteEntity, spriteComponent);

    controller::InputState input{};
    input.controlHeld = true;
    input.mouseLeftPressed = true;
    input.mouseGridX = 0.0f;
    input.mouseGridY = 0.0f;

    system.update(registry, input, true, debugSession);

    REQUIRE_FALSE(debugSession.selectedEntity.has_value());
    REQUIRE_FALSE(registry.getComponent<view::Sprite>(spriteEntity).isSelected);
}

TEST_CASE_METHOD(TestFixture, "DebugSelectionSystem clears selected sprite when debug is disabled")
{
    game::LocationTable unused({0, 0}, {0, 0});
    game::Registry registry;
    game::DebugSelectionSystem system;
    game::GameDebugSession debugSession(registry, unused);

    game::Entity spriteEntity = registry.createEntity();
    view::Sprite spriteComponent{
        .width = 32.0f,
        .height = 32.0f,
        .isSelected = true,
    };

    registry.addComponent(spriteEntity, spriteComponent);
    debugSession.selectedEntity = spriteEntity;

    controller::InputState input{};

    system.update(registry, input, false, debugSession);

    REQUIRE_FALSE(registry.getComponent<view::Sprite>(spriteEntity).isSelected);
}