#include "game/ecs/components/position.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/movement_system.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("MovementSystem updates position using velocity and dt")
{
    // ARRANGE
    game::Registry registry;
    game::MovementSystem system;

    game::Entity e = registry.createEntity();
    registry.addComponent<game::Position>(e, {10.0f, 20.0f});
    registry.addComponent<game::Velocity>(e, {4.0f, -6.0f});

    // ACT
    float dt = 0.5f;
    system.update(registry, dt);

    // ASSERT
    const auto &position = registry.getComponent<game::Position>(e);
    REQUIRE(position.x == (10.0f + 4.0f * dt));
    REQUIRE(position.y == (20.0f - 6.0f * dt));
}

TEST_CASE("MovementSystem does not touch entity without Velocity")
{
    // ARRANGE
    game::Registry registry;
    game::MovementSystem system;

    game::Entity e = registry.createEntity();
    registry.addComponent<game::Position>(e, {10.0f, 20.0f});

    // ACT
    float dt = 1.0f;
    system.update(registry, dt);

    // ASSERT
    const auto &position = registry.getComponent<game::Position>(e);
    REQUIRE(position.x == 10.0f);
    REQUIRE(position.y == 20.0f);
}

TEST_CASE("MovementSystem does not touch entity without Position")
{
    // ARRANGE
    game::Registry registry;
    game::MovementSystem system;

    game::Entity e = registry.createEntity();
    registry.addComponent<game::Velocity>(e, {4.0f, 5.0f});

    // ACT & ASSERT
    float dt = 1.0f;
    REQUIRE_NOTHROW(system.update(registry, dt));
}

TEST_CASE("MovementSystem updates multiple matching entities")
{
    // ARRANGE
    game::Registry registry;
    game::MovementSystem system;

    game::Entity e1 = registry.createEntity();
    registry.addComponent<game::Position>(e1, {0.0f, 0.0f});
    registry.addComponent<game::Velocity>(e1, {1.0f, 2.0f});

    game::Entity e2 = registry.createEntity();
    registry.addComponent<game::Position>(e2, {10.0f, 10.0f});
    registry.addComponent<game::Velocity>(e2, {-3.0f, 4.0f});

    // ACT
    float dt = 2.0f;
    system.update(registry, dt);

    // ASSERT
    REQUIRE(registry.getComponent<game::Position>(e1).x == (0.0f + 1.0f * dt));
    REQUIRE(registry.getComponent<game::Position>(e1).y == (0.0f + 2.0f * dt));

    REQUIRE(registry.getComponent<game::Position>(e2).x == (10.0f - 3.0f * dt));
    REQUIRE(registry.getComponent<game::Position>(e2).y == (10.0f + 4.0f * dt));
}