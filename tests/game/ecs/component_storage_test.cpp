#include "game/ecs/component_storage.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/entity.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("ComponentStorage adds and returns a component")
{
    // ARRANGE
    game::Entity entity = 1;
    game::ComponentStorage<game::Position> storage;

    // ACT
    storage.addComponent(entity, {10.0f, 20.0f});

    // ASSERT
    REQUIRE(storage.hasComponent(entity));
    REQUIRE(storage.getComponent(entity).x == 10.0f);
    REQUIRE(storage.getComponent(entity).y == 20.0f);
}

TEST_CASE("ComponentStorage throws when adding duplicate component")
{
    // ARRANGE
    game::Entity entity = 1;
    game::ComponentStorage<game::Position> storage;

    // ACT
    storage.addComponent(entity, {10.0f, 20.0f});

    // ASSERT
    REQUIRE_THROWS(storage.addComponent(entity, {30.0f, 40.0f}));
}

TEST_CASE("ComponentStorage removeComponent removes existing component")
{
    // ARRANGE
    game::Entity entity = 1;
    game::ComponentStorage<game::Position> storage;
    storage.addComponent(entity, {10.0f, 20.0f});

    // ACT
    storage.removeComponent(entity);

    // ASSERT
    REQUIRE_FALSE(storage.hasComponent(entity));
    REQUIRE_THROWS(storage.getComponent(entity));
}

TEST_CASE("ComponentStorage removeComponent on missing entity does nothing")
{
    // ARRANGE
    game::Entity nonExistingEntity = 42;
    game::ComponentStorage<game::Position> storage;

    // ACT & ASSERT
    REQUIRE_NOTHROW(storage.removeComponent(nonExistingEntity));
}

TEST_CASE("ComponentStorage keeps remaining components accessible after compacting")
{
    // ARRANGE
    game::Entity entity1 = 1;
    game::Entity entity2 = 2;
    game::Entity entity3 = 3;
    game::ComponentStorage<game::Position> storage;

    storage.addComponent(entity1, {1.0f, 1.0f});
    storage.addComponent(entity2, {2.0f, 2.0f});
    storage.addComponent(entity3, {3.0f, 3.0f});

    // ACT
    storage.removeComponent(entity2);

    // ASSERT
    REQUIRE(storage.hasComponent(entity1));
    REQUIRE(storage.hasComponent(entity3));
    REQUIRE_FALSE(storage.hasComponent(entity2));

    REQUIRE(storage.getComponent(entity1).x == 1.0f);
    REQUIRE(storage.getComponent(entity3).x == 3.0f);
}