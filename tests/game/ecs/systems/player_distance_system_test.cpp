#include "game/ecs/components/distance_to_player.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/player_distance_system.hpp"
#include "shared/test_fixture.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE_METHOD(TestFixture, "PlayerDistanceSystem computes euclidean distance for enemies")
{
    game::Registry registry;
    game::PlayerDistanceSystem system;

    const game::Entity player = registry.createEntity();
    registry.addComponent<game::PlayerTag>(player, {});
    registry.addComponent<game::Position>(player, {{0.0f, 0.0f}});

    const game::Entity enemy = registry.createEntity();
    registry.addComponent<game::EnemyTag>(enemy, {});
    registry.addComponent<game::Position>(enemy, {{30.0f, 40.0f}});

    system.update(registry);

    REQUIRE(registry.hasComponent<game::DistanceToPlayer>(enemy));
    const auto &distance = registry.getComponent<game::DistanceToPlayer>(enemy);
    REQUIRE(distance.hasPlayer);
    REQUIRE(distance.value == Catch::Approx(50.0f));
}

TEST_CASE_METHOD(TestFixture, "PlayerDistanceSystem marks enemies as playerless when no player exists")
{
    game::Registry registry;
    game::PlayerDistanceSystem system;

    const game::Entity enemy = registry.createEntity();
    registry.addComponent<game::EnemyTag>(enemy, {});
    registry.addComponent<game::Position>(enemy, {{10.0f, 20.0f}});

    system.update(registry);

    REQUIRE(registry.hasComponent<game::DistanceToPlayer>(enemy));
    const auto &distance = registry.getComponent<game::DistanceToPlayer>(enemy);
    REQUIRE_FALSE(distance.hasPlayer);
}

TEST_CASE_METHOD(TestFixture, "PlayerDistanceSystem updates multiple enemies in one tick")
{
    game::Registry registry;
    game::PlayerDistanceSystem system;

    const game::Entity player = registry.createEntity();
    registry.addComponent<game::PlayerTag>(player, {});
    registry.addComponent<game::Position>(player, {{100.0f, 100.0f}});

    const game::Entity enemyA = registry.createEntity();
    registry.addComponent<game::EnemyTag>(enemyA, {});
    registry.addComponent<game::Position>(enemyA, {{100.0f, 100.0f}});

    const game::Entity enemyB = registry.createEntity();
    registry.addComponent<game::EnemyTag>(enemyB, {});
    registry.addComponent<game::Position>(enemyB, {{160.0f, 100.0f}});

    system.update(registry);

    const auto &distanceA = registry.getComponent<game::DistanceToPlayer>(enemyA);
    const auto &distanceB = registry.getComponent<game::DistanceToPlayer>(enemyB);

    REQUIRE(distanceA.hasPlayer);
    REQUIRE(distanceA.value == Catch::Approx(0.0f));
    REQUIRE(distanceB.hasPlayer);
    REQUIRE(distanceB.value == Catch::Approx(60.0f));
}
