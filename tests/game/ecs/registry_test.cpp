#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/ecs/registry.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Registry creates entities")
{
    // ARRANGE
    game::Registry registry;

    // ACT
    game::Entity e1 = registry.createEntity();
    game::Entity e2 = registry.createEntity();

    // ASSERT
    REQUIRE(e1 != e2);
    REQUIRE(registry.entities().size() == 2);
}

TEST_CASE("Registry reuses destroyed entity ids")
{
    // ARRANGE
    game::Registry registry;
    registry.createEntity(); // creates e1
    game::Entity e2 = registry.createEntity();
    registry.destroyEntity(e2);

    // ACT
    game::Entity e3 = registry.createEntity();

    // ASSERT
    REQUIRE(e3 == e2);
    REQUIRE(registry.entities().size() == 2);
}

TEST_CASE("Registry add/get/has component works")
{
    // ARRANGE
    game::Registry registry;
    game::Entity e = registry.createEntity();

    // ACT
    registry.addComponent<game::Position>(e, {5.0f, 7.0f});

    // ASSERT
    REQUIRE(registry.hasComponent<game::Position>(e));
    REQUIRE(registry.getComponent<game::Position>(e).x == 5.0f);
    REQUIRE(registry.getComponent<game::Position>(e).y == 7.0f);
}

TEST_CASE("Registry addComponent throws for non-existing entity")
{
    // ARRANGE
    game::Registry registry;
    game::Entity nonExistingEntity = 42;

    // ACT & ASSERT
    REQUIRE_THROWS(registry.addComponent<game::Position>(nonExistingEntity, {1.0f, 2.0f}));
}

TEST_CASE("Registry removeComponent removes component")
{
    // ARRANGE
    game::Registry registry;
    game::Entity e = registry.createEntity();
    registry.addComponent<game::Position>(e, {1.0f, 2.0f});

    // ACT
    registry.removeComponent<game::Position>(e);

    // ASSERT
    REQUIRE_FALSE(registry.hasComponent<game::Position>(e));
}

TEST_CASE("Registry destroyEntity removes all components")
{
    // ARRANGE
    game::Registry registry;
    game::Entity e = registry.createEntity();

    registry.addComponent<game::Position>(e, {1.0f, 2.0f});
    registry.addComponent<game::Velocity>(e, {3.0f, 4.0f});

    // ACT
    registry.destroyEntity(e);

    // ASSERT
    REQUIRE_FALSE(registry.hasComponent<game::Position>(e));
    REQUIRE_FALSE(registry.hasComponent<game::Velocity>(e));
    REQUIRE(registry.entities().empty());
}

TEST_CASE("Registry view returns only entities with all requested components")
{
    // ARRANGE
    game::Registry registry;
    game::Entity e1 = registry.createEntity();
    game::Entity e2 = registry.createEntity();
    game::Entity e3 = registry.createEntity();

    registry.addComponent<game::Position>(e1, {1.0f, 1.0f});
    registry.addComponent<game::Velocity>(e1, {10.0f, 0.0f});
    registry.addComponent<game::Position>(e2, {2.0f, 2.0f});
    registry.addComponent<game::Velocity>(e3, {30.0f, 0.0f});

    // ACT
    auto result = registry.view<game::Position, game::Velocity>();

    // ASSERT
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == e1);
}