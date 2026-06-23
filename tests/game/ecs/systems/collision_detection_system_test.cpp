#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/enemy_attack_tag.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/player_attack_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/collision_detection_system.hpp"
#include "shared/test_fixture.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace {

game::HitBox makeHitBox(float offsetX, float offsetY, float width, float height)
{
    game::HitBox hitBox{};
    hitBox.offset = {offsetX, offsetY};
    hitBox.size = {width, height};
    return hitBox;
}

game::Entity addEntityWithHitBox(game::Registry &registry, float x, float y, float offsetX, float offsetY, float width,
                                 float height)
{
    const game::Entity entity = registry.createEntity();

    registry.addComponent<game::Position>(entity, {x, y});
    registry.addComponent<game::HitBox>(entity, makeHitBox(offsetX, offsetY, width, height));

    return entity;
}

game::Damage makeMeleeDamage(float amount = 1.0f)
{
    return game::Damage{
        .amount = amount,
        .pushBackForce = 0.0f,
        .stunChance = 0.0f,
        .kind = game::DamageKind::MeleeArc,
        .params =
            game::MeleeArcDamage{
                .reach = 20.0f,
                .activeTimeSec = 0.2f,
                .elapsedSec = 0.0f,
            },
    };
}

game::Damage makeProjectileDamage(float amount = 1.0f)
{
    return game::Damage{
        .amount = amount,
        .pushBackForce = 0.0f,
        .stunChance = 0.0f,
        .kind = game::DamageKind::Projectile,
        .params =
            game::ProjectileDamage{
                .speed = 0.0f,
                .maxRange = 0.0f,
                .distanceTraveled = 0.0f,
                .maxTargets = 1,
            },
    };
}

} // namespace

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem update activates player melee damage against enemy")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    const game::Entity attack = addEntityWithHitBox(registry, 0.0f, 0.0f, 0.0f, 0.0f, 10.0f, 10.0f);

    registry.addComponent<game::Damage>(attack, makeMeleeDamage(12.0f));
    registry.addComponent<game::PlayerAttackTag>(attack, {});

    const game::Entity enemy = addEntityWithHitBox(registry, 5.0f, 5.0f, 0.0f, 0.0f, 10.0f, 10.0f);

    registry.addComponent<game::EnemyTag>(enemy, {});

    system.update(registry);

    REQUIRE(registry.hasComponent<game::DamageTag>(attack));

    const auto &damageTag = registry.getComponent<game::DamageTag>(attack);
    REQUIRE(damageTag.targets.contains(enemy));
}

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem update activates player projectile damage against enemy")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    const game::Entity attack = addEntityWithHitBox(registry, 0.0f, 0.0f, 0.0f, 0.0f, 10.0f, 10.0f);

    registry.addComponent<game::Damage>(attack, makeProjectileDamage(12.0f));
    registry.addComponent<game::PlayerAttackTag>(attack, {});

    const game::Entity enemy = addEntityWithHitBox(registry, 5.0f, 5.0f, 0.0f, 0.0f, 10.0f, 10.0f);

    registry.addComponent<game::EnemyTag>(enemy, {});

    system.update(registry);

    REQUIRE(registry.hasComponent<game::DamageTag>(attack));

    const auto &damageTag = registry.getComponent<game::DamageTag>(attack);
    REQUIRE(damageTag.targets.contains(enemy));
}

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem update does not activate damage when hitboxes do not overlap")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    const game::Entity attack = addEntityWithHitBox(registry, 0.0f, 0.0f, 0.0f, 0.0f, 10.0f, 10.0f);

    registry.addComponent<game::Damage>(attack, makeMeleeDamage());
    registry.addComponent<game::PlayerAttackTag>(attack, {});

    const game::Entity enemy = addEntityWithHitBox(registry, 50.0f, 50.0f, 0.0f, 0.0f, 10.0f, 10.0f);

    registry.addComponent<game::EnemyTag>(enemy, {});

    system.update(registry);

    REQUIRE_FALSE(registry.hasComponent<game::DamageTag>(attack));
}

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem update detects collision using hitbox offset")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    const game::Entity attack = addEntityWithHitBox(registry, 0.0f, 0.0f, 5.0f, 0.0f, 10.0f, 10.0f);

    registry.addComponent<game::Damage>(attack, makeMeleeDamage());
    registry.addComponent<game::PlayerAttackTag>(attack, {});

    const game::Entity enemy = addEntityWithHitBox(registry, 14.0f, 0.0f, 0.0f, 0.0f, 10.0f, 10.0f);

    registry.addComponent<game::EnemyTag>(enemy, {});

    system.update(registry);

    REQUIRE(registry.hasComponent<game::DamageTag>(attack));

    const auto &damageTag = registry.getComponent<game::DamageTag>(attack);
    REQUIRE(damageTag.targets.contains(enemy));
}

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem update collects multiple enemy targets for one player attack")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    const game::Entity attack = addEntityWithHitBox(registry, 0.0f, 0.0f, 0.0f, 0.0f, 20.0f, 20.0f);

    registry.addComponent<game::Damage>(attack, makeMeleeDamage());
    registry.addComponent<game::PlayerAttackTag>(attack, {});

    const game::Entity enemyA = addEntityWithHitBox(registry, 5.0f, 5.0f, 0.0f, 0.0f, 10.0f, 10.0f);

    const game::Entity enemyB = addEntityWithHitBox(registry, 10.0f, 10.0f, 0.0f, 0.0f, 10.0f, 10.0f);

    registry.addComponent<game::EnemyTag>(enemyA, {});
    registry.addComponent<game::EnemyTag>(enemyB, {});

    system.update(registry);

    REQUIRE(registry.hasComponent<game::DamageTag>(attack));

    const auto &damageTag = registry.getComponent<game::DamageTag>(attack);
    REQUIRE(damageTag.targets.size() == 2);
    REQUIRE(damageTag.targets.contains(enemyA));
    REQUIRE(damageTag.targets.contains(enemyB));
}

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem update does not activate player attack against player")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    const game::Entity attack = addEntityWithHitBox(registry, 0.0f, 0.0f, 0.0f, 0.0f, 10.0f, 10.0f);

    registry.addComponent<game::Damage>(attack, makeMeleeDamage());
    registry.addComponent<game::PlayerAttackTag>(attack, {});

    const game::Entity player = addEntityWithHitBox(registry, 5.0f, 5.0f, 0.0f, 0.0f, 10.0f, 10.0f);

    registry.addComponent<game::PlayerTag>(player, {});

    system.update(registry);

    REQUIRE_FALSE(registry.hasComponent<game::DamageTag>(attack));
}

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem update does not activate enemy attack against enemy")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    const game::Entity attack = addEntityWithHitBox(registry, 0.0f, 0.0f, 0.0f, 0.0f, 10.0f, 10.0f);

    registry.addComponent<game::Damage>(attack, makeMeleeDamage());
    registry.addComponent<game::EnemyAttackTag>(attack, {});

    const game::Entity enemy = addEntityWithHitBox(registry, 5.0f, 5.0f, 0.0f, 0.0f, 10.0f, 10.0f);

    registry.addComponent<game::EnemyTag>(enemy, {});

    system.update(registry);

    REQUIRE_FALSE(registry.hasComponent<game::DamageTag>(attack));
}

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem update does not activate damage against another damage entity")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    const game::Entity attackA = addEntityWithHitBox(registry, 0.0f, 0.0f, 0.0f, 0.0f, 10.0f, 10.0f);

    registry.addComponent<game::Damage>(attackA, makeMeleeDamage());
    registry.addComponent<game::PlayerAttackTag>(attackA, {});

    const game::Entity attackB = addEntityWithHitBox(registry, 5.0f, 5.0f, 0.0f, 0.0f, 10.0f, 10.0f);

    registry.addComponent<game::Damage>(attackB, makeProjectileDamage());
    registry.addComponent<game::EnemyAttackTag>(attackB, {});

    system.update(registry);

    REQUIRE_FALSE(registry.hasComponent<game::DamageTag>(attackA));
    REQUIRE_FALSE(registry.hasComponent<game::DamageTag>(attackB));
}

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem update does not duplicate existing damage targets")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    const game::Entity attack = addEntityWithHitBox(registry, 0.0f, 0.0f, 0.0f, 0.0f, 10.0f, 10.0f);

    registry.addComponent<game::Damage>(attack, makeMeleeDamage());
    registry.addComponent<game::PlayerAttackTag>(attack, {});

    const game::Entity enemy = addEntityWithHitBox(registry, 5.0f, 5.0f, 0.0f, 0.0f, 10.0f, 10.0f);

    registry.addComponent<game::EnemyTag>(enemy, {});

    system.update(registry);
    system.update(registry);

    REQUIRE(registry.hasComponent<game::DamageTag>(attack));

    const auto &damageTag = registry.getComponent<game::DamageTag>(attack);
    REQUIRE(damageTag.targets.size() == 1);
    REQUIRE(damageTag.targets.contains(enemy));
}

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem update keeps entity inside map bounds")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    const game::Entity map = addEntityWithHitBox(registry, 0.0f, 0.0f, 0.0f, 0.0f, 100.0f, 100.0f);

    registry.addComponent<game::MapTag>(map, {});

    const game::Entity entity = addEntityWithHitBox(registry, -10.0f, 95.0f, 0.0f, 0.0f, 20.0f, 20.0f);

    system.update(registry);

    const auto &position = registry.getComponent<game::Position>(entity).p;

    REQUIRE(position.x == Catch::Approx(0.0f));
    REQUIRE(position.y == Catch::Approx(80.0f));
}

TEST_CASE_METHOD(TestFixture,
                 "CollisionDetectionSystem update keeps entity inside map bounds while respecting hitbox offset")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    const game::Entity map = addEntityWithHitBox(registry, 0.0f, 0.0f, 0.0f, 0.0f, 100.0f, 100.0f);

    registry.addComponent<game::MapTag>(map, {});

    const game::Entity entity = addEntityWithHitBox(registry, -15.0f, 95.0f, 5.0f, -5.0f, 20.0f, 20.0f);

    system.update(registry);

    const auto &position = registry.getComponent<game::Position>(entity).p;

    REQUIRE(position.x == Catch::Approx(-5.0f));
    REQUIRE(position.y == Catch::Approx(85.0f));
}

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem update does not enforce map bounds for damage entities")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    const game::Entity map = addEntityWithHitBox(registry, 0.0f, 0.0f, 0.0f, 0.0f, 100.0f, 100.0f);

    registry.addComponent<game::MapTag>(map, {});

    const game::Entity damage = addEntityWithHitBox(registry, -10.0f, 95.0f, 0.0f, 0.0f, 20.0f, 20.0f);

    registry.addComponent<game::Damage>(damage, makeMeleeDamage());
    registry.addComponent<game::PlayerAttackTag>(damage, {});

    system.update(registry);

    const auto &position = registry.getComponent<game::Position>(damage).p;

    REQUIRE(position.x == Catch::Approx(-10.0f));
    REQUIRE(position.y == Catch::Approx(95.0f));
}

TEST_CASE_METHOD(TestFixture, "CollisionDetectionSystem update does not change position when no map exists")
{
    game::Registry registry;
    game::CollisionDetectionSystem system;

    const game::Entity entity = addEntityWithHitBox(registry, -10.0f, 95.0f, 0.0f, 0.0f, 20.0f, 20.0f);

    system.update(registry);

    const auto &position = registry.getComponent<game::Position>(entity).p;

    REQUIRE(position.x == Catch::Approx(-10.0f));
    REQUIRE(position.y == Catch::Approx(95.0f));
}