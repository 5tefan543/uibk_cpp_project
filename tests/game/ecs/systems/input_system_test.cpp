#include "controller/input/input_state.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/input_system.hpp"
#include "shared/test_fixture.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE_METHOD(TestFixture, "InputSystem sets player velocity from input")
{
    // ARRANGE
    game::Registry registry;
    game::InputSystem system;

    game::Entity player = registry.createEntity();
    game::PlayerStats playerStats;
    playerStats.moveSpeed = 200.0f;
    registry.addComponent<game::PlayerStats>(player, playerStats);
    registry.addComponent<game::Velocity>(player, {0.0f, 0.0f});

    controller::InputState input;
    input.leftHeld = true;
    input.upHeld = true;

    // ACT
    system.update(registry, input);

    // ASSERT
    const auto &velocity = registry.getComponent<game::Velocity>(player);
    REQUIRE(velocity.dx == -200.0f);
    REQUIRE(velocity.dy == -200.0f);
}

TEST_CASE_METHOD(TestFixture, "InputSystem resets old velocity before applying new input")
{
    // ARRANGE
    game::Registry registry;
    game::InputSystem system;

    game::Entity player = registry.createEntity();
    game::PlayerStats playerStats;
    playerStats.moveSpeed = 200.0f;
    registry.addComponent<game::PlayerStats>(player, playerStats);
    registry.addComponent<game::Velocity>(player, {999.0f, 999.0f});

    controller::InputState input;

    // ACT
    system.update(registry, input);

    // ASSERT
    const auto &velocity = registry.getComponent<game::Velocity>(player);
    REQUIRE(velocity.dx == 0.0f);
    REQUIRE(velocity.dy == 0.0f);
}

TEST_CASE_METHOD(TestFixture, "InputSystem does not update entity without PlayerStats")
{
    // ARRANGE
    game::Registry registry;
    game::InputSystem system;

    game::Entity notPlayer = registry.createEntity();
    registry.addComponent<game::Velocity>(notPlayer, {5.0f, 6.0f});

    controller::InputState input;
    input.rightHeld = true;
    input.downHeld = true;

    // ACT
    system.update(registry, input);

    // ASSERT
    const auto &velocity = registry.getComponent<game::Velocity>(notPlayer);
    REQUIRE(velocity.dx == 5.0f);
    REQUIRE(velocity.dy == 6.0f);
}

TEST_CASE_METHOD(TestFixture, "InputSystem opposing directions cancel each other")
{
    // ARRANGE
    game::Registry registry;
    game::InputSystem system;

    game::Entity player = registry.createEntity();
    game::PlayerStats playerStats;
    playerStats.moveSpeed = 200.0f;
    registry.addComponent<game::PlayerStats>(player, playerStats);
    registry.addComponent<game::Velocity>(player, {0.0f, 0.0f});

    controller::InputState input;
    input.leftHeld = true;
    input.rightHeld = true;
    input.upHeld = true;
    input.downHeld = true;

    // ACT
    system.update(registry, input);

    // ASSERT
    const auto &velocity = registry.getComponent<game::Velocity>(player);
    REQUIRE(velocity.dx == 0.0f);
    REQUIRE(velocity.dy == 0.0f);
}