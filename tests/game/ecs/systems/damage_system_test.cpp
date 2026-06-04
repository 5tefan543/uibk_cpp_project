#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/damage_system.hpp"
#include "shared/test_fixture.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE_METHOD(TestFixture, "DamageSystem destroys projectile after max range")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity projectile = registry.createEntity();
    registry.addComponent<game::Velocity>(projectile, {6.0f, 8.0f}); // speed magnitude = 10
    registry.addComponent<game::Damage>(
        projectile, {.amount = 5.0f,
                     .isMultiHit = false,
                     .pushBackForce = 0.0f,
                     .stunChance = 0.0f,
                     .kind = game::DamageKind::Projectile,
                     .params = game::ProjectileDamage{
                         .speed = 10.0f, .maxRange = 5.0f, .distanceTraveled = 0.0f, .targetsHit = 0}});
    const game::Entity enemy = registry.createEntity();
    game::EnemyStats enemyStats;
    enemyStats.health = 100.0f;
    enemyStats.scoreReward = 1;
    registry.addComponent<game::EnemyStats>(enemy, enemyStats);
    registry.addComponent<game::DamageTag>(projectile, {.targets = {enemy}});

    system.update(registry, 1.0f);

    REQUIRE_FALSE(registry.isEntityAlive(projectile));
}

TEST_CASE_METHOD(TestFixture, "DamageSystem tracks projectile traveled distance")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity projectile = registry.createEntity();
    registry.addComponent<game::Velocity>(projectile, {3.0f, 4.0f}); // speed magnitude = 5
    registry.addComponent<game::Damage>(
        projectile, {.amount = 5.0f,
                     .isMultiHit = false,
                     .pushBackForce = 0.0f,
                     .stunChance = 0.0f,
                     .kind = game::DamageKind::Projectile,
                     .params = game::ProjectileDamage{
                         .speed = 5.0f, .maxRange = 25.0f, .distanceTraveled = 0.0f, .targetsHit = 0}});
    const game::Entity enemy = registry.createEntity();
    game::EnemyStats enemyStats;
    enemyStats.health = 100.0f;
    enemyStats.scoreReward = 1;
    registry.addComponent<game::EnemyStats>(enemy, enemyStats);
    registry.addComponent<game::DamageTag>(projectile, {.targets = {enemy}});

    system.update(registry, 2.0f);

    REQUIRE(registry.isEntityAlive(projectile));
    const auto &damage = registry.getComponent<game::Damage>(projectile);
    const auto *projectileParams = std::get_if<game::ProjectileDamage>(&damage.params);
    REQUIRE(projectileParams != nullptr);
    REQUIRE(projectileParams->distanceTraveled == 10.0f);
}

TEST_CASE_METHOD(TestFixture, "DamageSystem destroys melee damage entity after active time")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity meleeDamage = registry.createEntity();
    registry.addComponent<game::Damage>(
        meleeDamage, {.amount = 7.0f,
                      .isMultiHit = false,
                      .pushBackForce = 0.0f,
                      .stunChance = 0.0f,
                      .kind = game::DamageKind::MeleeArc,
                      .params = game::MeleeArcDamage{.reach = 40.0f, .activeTimeSec = 0.2f, .elapsedSec = 0.0f}});
    registry.addComponent<game::DamageTag>(meleeDamage, {});

    system.update(registry, 0.1f);
    REQUIRE(registry.isEntityAlive(meleeDamage));

    system.update(registry, 0.11f);
    REQUIRE_FALSE(registry.isEntityAlive(meleeDamage));
}

TEST_CASE_METHOD(TestFixture, "DamageSystem applies damage on collision and destroys damage entity")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity player = registry.createEntity();
    game::PlayerStats playerStats;
    playerStats.health = 100.0f;
    registry.addComponent<game::PlayerStats>(player, playerStats);

    const game::Entity enemy = registry.createEntity();
    game::EnemyStats enemyStats;
    enemyStats.health = 5.0f;
    enemyStats.scoreReward = 3;
    registry.addComponent<game::EnemyStats>(enemy, enemyStats);

    const game::Entity projectile = registry.createEntity();
    registry.addComponent<game::Damage>(
        projectile, {.amount = 10.0f,
                     .isMultiHit = false,
                     .pushBackForce = 0.0f,
                     .stunChance = 0.0f,
                     .kind = game::DamageKind::Projectile,
                     .params = game::ProjectileDamage{
                         .speed = 0.0f, .maxRange = 100.0f, .distanceTraveled = 0.0f, .targetsHit = 1}});
    registry.addComponent<game::DamageTag>(projectile, {.targets = {enemy}});

    system.update(registry, 0.016f);

    REQUIRE_FALSE(registry.isEntityAlive(projectile));
    REQUIRE_FALSE(registry.isEntityAlive(enemy));

    const auto &updatedPlayerStats = registry.getComponent<game::PlayerStats>(player);
    REQUIRE(updatedPlayerStats.score == 3);
    REQUIRE(updatedPlayerStats.currency == 3);
}

TEST_CASE_METHOD(TestFixture, "DamageSystem projectile uses configured speed when velocity component is absent")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity projectile = registry.createEntity();
    registry.addComponent<game::Damage>(
        projectile, {.amount = 5.0f,
                     .isMultiHit = false,
                     .pushBackForce = 0.0f,
                     .stunChance = 0.0f,
                     .kind = game::DamageKind::Projectile,
                     .params = game::ProjectileDamage{
                         .speed = 7.0f, .maxRange = 100.0f, .distanceTraveled = 0.0f, .targetsHit = 0}});
    const game::Entity enemy = registry.createEntity();
    game::EnemyStats enemyStats;
    enemyStats.health = 100.0f;
    enemyStats.scoreReward = 1;
    registry.addComponent<game::EnemyStats>(enemy, enemyStats);
    registry.addComponent<game::DamageTag>(projectile, {.targets = {enemy}});

    system.update(registry, 3.0f);

    REQUIRE(registry.isEntityAlive(projectile));
    const auto &damage = registry.getComponent<game::Damage>(projectile);
    const auto *projectileParams = std::get_if<game::ProjectileDamage>(&damage.params);
    REQUIRE(projectileParams != nullptr);
    REQUIRE(projectileParams->distanceTraveled == 21.0f);
}

TEST_CASE_METHOD(TestFixture, "DamageSystem destroys projectile when all tagged targets are dead")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity projectile = registry.createEntity();
    registry.addComponent<game::Damage>(
        projectile, {.amount = 1.0f,
                     .isMultiHit = false,
                     .pushBackForce = 0.0f,
                     .stunChance = 0.0f,
                     .kind = game::DamageKind::Projectile,
                     .params = game::ProjectileDamage{
                         .speed = 0.0f, .maxRange = 100.0f, .distanceTraveled = 0.0f, .targetsHit = 0}});

    const game::Entity deadTarget = registry.createEntity();
    game::EnemyStats enemyStats;
    enemyStats.health = 10.0f;
    enemyStats.scoreReward = 1;
    registry.addComponent<game::EnemyStats>(deadTarget, enemyStats);

    registry.addComponent<game::DamageTag>(projectile, {.targets = {deadTarget}});
    registry.destroyEntity(deadTarget);

    system.update(registry, 0.016f);

    REQUIRE_FALSE(registry.isEntityAlive(projectile));
}

TEST_CASE_METHOD(TestFixture, "DamageSystem destroys beam and area damage entities after active time")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity beam = registry.createEntity();
    registry.addComponent<game::Damage>(
        beam, {.amount = 3.0f,
               .isMultiHit = false,
               .pushBackForce = 0.0f,
               .stunChance = 0.0f,
               .kind = game::DamageKind::Beam,
               .params = game::BeamDamage{.length = 80.0f, .width = 12.0f, .activeTimeSec = 0.2f, .elapsedSec = 0.0f}});
    registry.addComponent<game::DamageTag>(beam, {});

    const game::Entity area = registry.createEntity();
    registry.addComponent<game::Damage>(
        area, {.amount = 3.0f,
               .isMultiHit = false,
               .pushBackForce = 0.0f,
               .stunChance = 0.0f,
               .kind = game::DamageKind::Area,
               .params = game::AreaDamage{.radius = 40.0f, .activeTimeSec = 0.4f, .elapsedSec = 0.0f}});
    registry.addComponent<game::DamageTag>(area, {});

    system.update(registry, 0.21f);
    REQUIRE_FALSE(registry.isEntityAlive(beam));
    REQUIRE(registry.isEntityAlive(area));

    system.update(registry, 0.2f);
    REQUIRE_FALSE(registry.isEntityAlive(area));
}