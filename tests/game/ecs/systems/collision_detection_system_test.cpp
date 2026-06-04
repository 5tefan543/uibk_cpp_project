#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
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

    system.update(registry, 1);

    REQUIRE(registry.hasComponent<game::HitBox>(entity));
    const auto &hitBox = registry.getComponent<game::HitBox>(entity);
    REQUIRE(hitBox.rect.x == 3.0f);
    REQUIRE(hitBox.rect.y == 7.0f);
    REQUIRE(hitBox.rect.width == 16.0f);
    REQUIRE(hitBox.rect.height == 20.0f);
}

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem update does nothing when entity has no Position")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    game::Entity entity = registry.createEntity();
    registry.addComponent<game::HitBox>(entity, {.rect = {1.0f, 2.0f, 3.0f, 4.0f}});

    system.update(registry, 1);

    const auto &hitBox = registry.getComponent<game::HitBox>(entity);
    REQUIRE(hitBox.rect.x == 1.0f);
    REQUIRE(hitBox.rect.y == 2.0f);
    REQUIRE(hitBox.rect.width == 3.0f);
    REQUIRE(hitBox.rect.height == 4.0f);
}

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem update does not update when entity has no hitbox")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    game::Entity entity = registry.createEntity();
    registry.addComponent<game::Position>(entity, {8.0f, 9.0f});

    system.update(registry, 1);

    REQUIRE(registry.hasComponent<game::Position>(entity));
    REQUIRE_FALSE(registry.hasComponent<game::HitBox>(entity));
}

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem activates player damage to enemy on collision")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    game::Entity player = registry.createEntity();
    registry.addComponent<game::PlayerTag>(player, {});
    registry.addComponent<game::Position>(player, {0.0f, 0.0f});
    registry.addComponent<view::Sprite>(player, {.width = 10.0f, .height = 10.0f});
    registry.addComponent<game::Damage>(
        player, game::Damage{.amount = 12.0f,
                             .pushBackForce = 0.0f,
                             .stunChance = 0.0f,
                             .kind = game::DamageKind::Projectile,
                             .params = game::ProjectileDamage{
                                 .speed = 0.0f, .maxRange = 0.0f, .distanceTraveled = 0.0f, .maxTargets = 1}});

    game::Entity enemy = registry.createEntity();
    registry.addComponent<game::EnemyTag>(enemy, {});
    game::EnemyStats enemyStats;
    enemyStats.health = 50.0f;
    enemyStats.scoreReward = 100;
    registry.addComponent<game::EnemyStats>(enemy, enemyStats);
    registry.addComponent<game::Position>(enemy, {5.0f, 5.0f});
    registry.addComponent<view::Sprite>(enemy, {.width = 10.0f, .height = 10.0f});
    REQUIRE_FALSE(registry.hasComponent<game::DamageTag>(player));

    system.update(registry, 1);
    system.update(registry, 1);

    const auto &updatedEnemyStats = registry.getComponent<game::EnemyStats>(enemy);
    REQUIRE(updatedEnemyStats.health == 50.0f);
    REQUIRE(registry.hasComponent<game::DamageTag>(player));
    const auto &damageTag = registry.getComponent<game::DamageTag>(player);
    REQUIRE(damageTag.targets.contains(enemy));
}

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem activatesto enemy damage on player on collision")
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
    registry.addComponent<game::Damage>(
        enemy, game::Damage{.amount = 7.0f,
                            .pushBackForce = 0.0f,
                            .stunChance = 0.0f,
                            .kind = game::DamageKind::MeleeArc,
                            .params = game::MeleeArcDamage{.reach = 20.0f, .activeTimeSec = 0.2f, .elapsedSec = 0.0f}});
    REQUIRE_FALSE(registry.hasComponent<game::DamageTag>(enemy));

    system.update(registry, 1);
    system.update(registry, 1);

    const auto &updatedPlayerStats = registry.getComponent<game::PlayerStats>(player);
    REQUIRE(updatedPlayerStats.health == 30.0f);
    REQUIRE(registry.hasComponent<game::DamageTag>(enemy));
    const auto &damageTag = registry.getComponent<game::DamageTag>(enemy);
    REQUIRE(damageTag.targets.contains(player));
}

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem keeps entities inside map bounds")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    game::Entity map = registry.createEntity();
    registry.addComponent<game::MapTag>(map, {});
    registry.addComponent<game::Position>(map, {0.0f, 0.0f});
    registry.addComponent<view::Sprite>(map, {.width = 100.0f, .height = 100.0f});

    game::Entity entity1 = registry.createEntity();
    registry.addComponent<game::Position>(entity1, {-10.0f, 95.0f});
    registry.addComponent<view::Sprite>(entity1, {.width = 20.0f, .height = 20.0f});

    game::Entity entity2 = registry.createEntity();
    registry.addComponent<game::Position>(entity2, {95.0f, -10.0f});
    registry.addComponent<view::Sprite>(entity2, {.width = 20.0f, .height = 20.0f});

    system.update(registry, 0);
    system.update(registry, 0);

    const auto &position = registry.getComponent<game::Position>(entity1);
    REQUIRE(position.x == 0.0f);
    REQUIRE(position.y == 80.0f);

    const auto &position2 = registry.getComponent<game::Position>(entity2);
    REQUIRE(position2.x == 80.0f);
    REQUIRE(position2.y == 0.0f);
}

TEST_CASE_METHOD(TestFixture, "Hitboxes are re-initialized when wave changes")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    game::Entity entity = registry.createEntity();
    registry.addComponent<game::Position>(entity, {3.0f, 7.0f});
    registry.addComponent<view::Sprite>(entity, {.width = 16.0f, .height = 20.0f});

    system.update(registry, 1);

    REQUIRE(registry.hasComponent<game::HitBox>(entity));
    const auto &hitBox = registry.getComponent<game::HitBox>(entity);
    REQUIRE(hitBox.rect.x == 3.0f);
    REQUIRE(hitBox.rect.y == 7.0f);
    REQUIRE(hitBox.rect.width == 16.0f);
    REQUIRE(hitBox.rect.height == 20.0f);

    game::Entity entity2 = registry.createEntity();
    registry.addComponent<game::Position>(entity2, {10.0f, 20.0f});
    registry.addComponent<view::Sprite>(entity2, {.width = 16.0f, .height = 20.0f});

    system.update(registry, 2);
    REQUIRE(registry.hasComponent<game::HitBox>(entity2));
    const auto &hitBox2 = registry.getComponent<game::HitBox>(entity2);
    REQUIRE(hitBox2.rect.x == 10.0f);
    REQUIRE(hitBox2.rect.y == 20.0f);
    REQUIRE(hitBox2.rect.width == 16.0f);
    REQUIRE(hitBox2.rect.height == 20.0f);
}

TEST_CASE_METHOD(TestFixture, "Hitboxes are not re-initialized when wave does not change")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    game::Entity entity = registry.createEntity();
    registry.addComponent<game::Position>(entity, {3.0f, 7.0f});
    registry.addComponent<view::Sprite>(entity, {.width = 16.0f, .height = 20.0f});

    system.update(registry, 1);

    REQUIRE(registry.hasComponent<game::HitBox>(entity));
    const auto &hitBox = registry.getComponent<game::HitBox>(entity);
    REQUIRE(hitBox.rect.x == 3.0f);
    REQUIRE(hitBox.rect.y == 7.0f);
    REQUIRE(hitBox.rect.width == 16.0f);
    REQUIRE(hitBox.rect.height == 20.0f);

    game::Entity entity2 = registry.createEntity();
    registry.addComponent<game::Position>(entity2, {10.0f, 20.0f});
    registry.addComponent<view::Sprite>(entity2, {.width = 16.0f, .height = 20.0f});

    system.update(registry, 1);
    REQUIRE_FALSE(registry.hasComponent<game::HitBox>(entity2));
}
