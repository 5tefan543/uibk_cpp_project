#include "game/debug/game_debug_session.hpp"
#include "game/ecs/components/camera.hpp"
#include "game/ecs/components/map.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/sprite.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/debug_selection_system.hpp"
#include "shared/test_fixture.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE_METHOD(TestFixture, "DebugSelectionSystem selects sprite under mouse when ctrl-clicked")
{
    game::Registry registry;
    game::DebugSelectionSystem system;
    game::GameDebugSession debugSession(registry);

    game::Entity spriteEntity = registry.createEntity();
    game::Position spritePosition{.x = 100.0f, .y = 100.0f};
    game::Sprite spriteComponent{.width = 32.0f, .height = 32.0f};
    registry.addComponent(spriteEntity, spritePosition);
    registry.addComponent(spriteEntity, spriteComponent);

    controller::InputState input{};
    input.controlHeld = true;
    input.mouseLeftPressed = true;
    input.mouseGridX = spritePosition.x;
    input.mouseGridY = spritePosition.y;

    system.update(registry, input, true, debugSession);

    REQUIRE(debugSession.selectedEntity == spriteEntity);
    REQUIRE(registry.getComponent<game::Sprite>(spriteEntity).isSelected);
}

TEST_CASE_METHOD(TestFixture, "DebugSelectionSystem selects map under mouse when ctrl-clicked")
{
    game::Registry registry;
    game::DebugSelectionSystem system;
    game::GameDebugSession debugSession(registry);

    game::Entity mapEntity = registry.createEntity();
    game::Map mapComponent{
        .x = 100.0f,
        .y = 100.0f,
        .width = 500.0f,
        .height = 500.0f,
    };
    registry.addComponent(mapEntity, mapComponent);

    controller::InputState input{};
    input.controlHeld = true;
    input.mouseLeftPressed = true;
    input.mouseGridX = mapComponent.x;
    input.mouseGridY = mapComponent.y;

    system.update(registry, input, true, debugSession);

    REQUIRE(debugSession.selectedEntity == mapEntity);
    REQUIRE(registry.getComponent<game::Map>(mapEntity).isSelected);
}

TEST_CASE_METHOD(TestFixture, "DebugSelectionSystem does not select entity when ctrl is not held")
{
    game::Registry registry;
    game::DebugSelectionSystem system;
    game::GameDebugSession debugSession(registry);

    game::Entity spriteEntity = registry.createEntity();
    game::Position spritePosition{.x = 100.0f, .y = 100.0f};
    game::Sprite spriteComponent{.width = 32.0f, .height = 32.0f};
    registry.addComponent(spriteEntity, spritePosition);
    registry.addComponent(spriteEntity, spriteComponent);

    controller::InputState input{};
    input.controlHeld = false;
    input.mouseLeftPressed = true;
    input.mouseGridX = spritePosition.x;
    input.mouseGridY = spritePosition.y;

    system.update(registry, input, true, debugSession);

    REQUIRE_FALSE(debugSession.selectedEntity.has_value());
    REQUIRE_FALSE(registry.getComponent<game::Sprite>(spriteEntity).isSelected);
}

TEST_CASE_METHOD(TestFixture, "DebugSelectionSystem does not select entity when mouse is outside entity")
{
    game::Registry registry;
    game::DebugSelectionSystem system;
    game::GameDebugSession debugSession(registry);

    game::Entity spriteEntity = registry.createEntity();
    game::Position spritePosition{.x = 100.0f, .y = 100.0f};
    game::Sprite spriteComponent{
        .width = 32.0f,
        .height = 32.0f,
    };
    registry.addComponent(spriteEntity, spritePosition);
    registry.addComponent(spriteEntity, spriteComponent);

    controller::InputState input{};
    input.controlHeld = true;
    input.mouseLeftPressed = true;
    input.mouseGridX = 0;
    input.mouseGridY = 0;

    system.update(registry, input, true, debugSession);

    REQUIRE_FALSE(debugSession.selectedEntity.has_value());
    REQUIRE_FALSE(registry.getComponent<game::Sprite>(spriteEntity).isSelected);
}

TEST_CASE_METHOD(TestFixture, "DebugSelectionSystem clears sprite selection when debug is disabled")
{
    game::Registry registry;
    game::DebugSelectionSystem system;
    game::GameDebugSession debugSession(registry);

    game::Entity spriteEntity = registry.createEntity();
    game::Sprite spriteComponent{
        .width = 32.0f,
        .height = 32.0f,
        .isSelected = true,
    };
    registry.addComponent(spriteEntity, spriteComponent);

    debugSession.selectedEntity = spriteEntity;

    controller::InputState input{};

    system.update(registry, input, false, debugSession);

    REQUIRE_FALSE(registry.getComponent<game::Sprite>(spriteEntity).isSelected);
}

TEST_CASE_METHOD(TestFixture, "DebugSelectionSystem clears map selection when debug is disabled")
{
    game::Registry registry;
    game::DebugSelectionSystem system;
    game::GameDebugSession debugSession(registry);

    game::Entity mapEntity = registry.createEntity();
    game::Map mapComponent{
        .x = 100.0f,
        .y = 100.0f,
        .width = 500.0f,
        .height = 500.0f,
        .isSelected = true,
    };
    registry.addComponent(mapEntity, mapComponent);

    debugSession.selectedEntity = mapEntity;

    controller::InputState input{};

    system.update(registry, input, false, debugSession);

    REQUIRE_FALSE(registry.getComponent<game::Map>(mapEntity).isSelected);
}