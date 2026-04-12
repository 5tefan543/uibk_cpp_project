#include "controller/input/input_state.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/input_system.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("InputSystem sets player velocity from input")
{
    // ARRANGE
    game::Registry registry;
    game::InputSystem system;

    game::Entity player = registry.createEntity();
    registry.addComponent<game::PlayerTag>(player, {});
    registry.addComponent<game::Velocity>(player, {0.0f, 0.0f});

    controller::InputState input;
    input.left = true;
    input.up = true;

    // ACT
    system.update(registry, input);

    // ASSERT
    const auto &velocity = registry.getComponent<game::Velocity>(player);
    REQUIRE(velocity.dx == -200.0f);
    REQUIRE(velocity.dy == -200.0f);
}

TEST_CASE("InputSystem resets old velocity before applying new input")
{
    // ARRANGE
    game::Registry registry;
    game::InputSystem system;

    game::Entity player = registry.createEntity();
    registry.addComponent<game::PlayerTag>(player, {});
    registry.addComponent<game::Velocity>(player, {999.0f, 999.0f});

    controller::InputState input;

    // ACT
    system.update(registry, input);

    // ASSERT
    const auto &velocity = registry.getComponent<game::Velocity>(player);
    REQUIRE(velocity.dx == 0.0f);
    REQUIRE(velocity.dy == 0.0f);
}

TEST_CASE("InputSystem does not update entity without PlayerTag")
{
    // ARRANGE
    game::Registry registry;
    game::InputSystem system;

    game::Entity notPlayer = registry.createEntity();
    registry.addComponent<game::Velocity>(notPlayer, {5.0f, 6.0f});

    controller::InputState input;
    input.right = true;
    input.down = true;

    // ACT
    system.update(registry, input);

    // ASSERT
    const auto &velocity = registry.getComponent<game::Velocity>(notPlayer);
    REQUIRE(velocity.dx == 5.0f);
    REQUIRE(velocity.dy == 6.0f);
}

TEST_CASE("InputSystem opposing directions cancel each other")
{
    // ARRANGE
    game::Registry registry;
    game::InputSystem system;

    game::Entity player = registry.createEntity();
    registry.addComponent<game::PlayerTag>(player, {});
    registry.addComponent<game::Velocity>(player, {0.0f, 0.0f});

    controller::InputState input;
    input.left = true;
    input.right = true;
    input.up = true;
    input.down = true;

    // ACT
    system.update(registry, input);

    // ASSERT
    const auto &velocity = registry.getComponent<game::Velocity>(player);
    REQUIRE(velocity.dx == 0.0f);
    REQUIRE(velocity.dy == 0.0f);
}