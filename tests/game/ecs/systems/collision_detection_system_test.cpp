#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/collision_detection_system.hpp"
#include "shared/test_fixture.hpp"
#include "view/sprite.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem initializes hitboxes on first update")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    game::Entity entity = registry.createEntity();
    registry.addComponent<game::Position>(entity, {3.0f, 7.0f});
    registry.addComponent<view::Sprite>(entity, {.width = 16.0f, .height = 20.0f});

    system.update(registry);

    REQUIRE(registry.hasComponent<game::HitBox>(entity));
    const auto &hitBox = registry.getComponent<game::HitBox>(entity);
    REQUIRE(hitBox.rect.x == 3.0f);
    REQUIRE(hitBox.rect.y == 7.0f);
    REQUIRE(hitBox.rect.width == 16.0f);
    REQUIRE(hitBox.rect.height == 20.0f);
    REQUIRE(hitBox.isActive);
}

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem applies player damage to enemy on collision")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    game::Entity player = registry.createEntity();
    registry.addComponent<game::PlayerTag>(player, {});
    registry.addComponent<game::Position>(player, {0.0f, 0.0f});
    registry.addComponent<view::Sprite>(player, {.width = 10.0f, .height = 10.0f});
    registry.addComponent<Damage>(
        player,
        {.amount = 12.0f,
         .isActive = true,
         .kind = DamageKind::Projectile,
         .params = ProjectileDamage{
             .speed = 0.0f, .maxRange = 0.0f, .distanceTraveled = 0.0f, .pushbackForce = 0.0f, .targetsHit = 0}});

    game::Entity enemy = registry.createEntity();
    registry.addComponent<game::EnemyTag>(enemy, {});
    game::EnemyStats enemyStats;
    enemyStats.health = 50.0f;
    enemyStats.scoreReward = 100;
    registry.addComponent<game::EnemyStats>(enemy, enemyStats);
    registry.addComponent<game::Position>(enemy, {5.0f, 5.0f});
    registry.addComponent<view::Sprite>(enemy, {.width = 10.0f, .height = 10.0f});

    system.update(registry);
    system.update(registry);

    const auto &updatedEnemyStats = registry.getComponent<game::EnemyStats>(enemy);
    REQUIRE(updatedEnemyStats.health == 38.0f);
}

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem applies enemy damage to player on collision")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    game::Entity player = registry.createEntity();
    registry.addComponent<game::PlayerTag>(player, {});
    game::PlayerStats playerStats;
    playerStats.health = 30.0f;
    registry.addComponent<game::PlayerStats>(player, playerStats);
    registry.addComponent<game::Position>(player, {0.0f, 0.0f});
    registry.addComponent<view::Sprite>(player, {.width = 10.0f, .height = 10.0f});

    game::Entity enemy = registry.createEntity();
    registry.addComponent<game::EnemyTag>(enemy, {});
    registry.addComponent<game::Position>(enemy, {4.0f, 4.0f});
    registry.addComponent<view::Sprite>(enemy, {.width = 10.0f, .height = 10.0f});
    registry.addComponent<Damage>(
        enemy, {.amount = 7.0f,
                .isActive = true,
                .kind = DamageKind::MeleeArc,
                .params = MeleeArcDamage{
                    .arcAngleDeg = 90.0f, .arcRadius = 20.0f, .activeTimeSec = 0.2f, .elapsedSec = 0.0f}});

    system.update(registry);
    system.update(registry);

    const auto &updatedPlayerStats = registry.getComponent<game::PlayerStats>(player);
    REQUIRE(updatedPlayerStats.health == 23.0f);
}

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem keeps entities inside map bounds")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    game::Entity map = registry.createEntity();
    registry.addComponent<game::MapTag>(map, {});
    registry.addComponent<game::Position>(map, {0.0f, 0.0f});
    registry.addComponent<view::Sprite>(map, {.width = 100.0f, .height = 100.0f});

    game::Entity entity = registry.createEntity();
    registry.addComponent<game::Position>(entity, {-10.0f, 95.0f});
    registry.addComponent<view::Sprite>(entity, {.width = 20.0f, .height = 20.0f});

    system.update(registry);
    system.update(registry);

    const auto &position = registry.getComponent<game::Position>(entity);
    REQUIRE(position.x == 0.0f);
    REQUIRE(position.y == 80.0f);
}
