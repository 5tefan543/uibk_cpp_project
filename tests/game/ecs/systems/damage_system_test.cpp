#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/damage_system.hpp"
#include "shared/test_fixture.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE_METHOD(TestFixture, "DamageSystem destroys projectile after max range")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity projectile = registry.createEntity();
    registry.addComponent<game::Velocity>(projectile, {6.0f, 8.0f}); // speed magnitude = 10
    registry.addComponent<game::Damage>(
        projectile, {.amount = 5.0f,
                     .pushBackForce = 0.0f,
                     .stunChance = 0.0f,
                     .kind = game::DamageKind::Projectile,
                     .params = game::ProjectileDamage{
                         .speed = 10.0f, .maxRange = 5.0f, .distanceTraveled = 0.0f, .maxTargets = 1}});
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
                     .pushBackForce = 0.0f,
                     .stunChance = 0.0f,
                     .kind = game::DamageKind::Projectile,
                     .params = game::ProjectileDamage{
                         .speed = 5.0f, .maxRange = 25.0f, .distanceTraveled = 0.0f, .maxTargets = 2}});
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
                     .pushBackForce = 0.0f,
                     .stunChance = 0.0f,
                     .kind = game::DamageKind::Projectile,
                     .params = game::ProjectileDamage{
                         .speed = 0.0f, .maxRange = 100.0f, .distanceTraveled = 0.0f, .maxTargets = 1}});
    registry.addComponent<game::DamageTag>(projectile, {.targets = {enemy}});

    system.update(registry, 0.016f);

    REQUIRE_FALSE(registry.isEntityAlive(projectile));
    REQUIRE_FALSE(registry.isEntityAlive(enemy));

    const auto &updatedPlayerStats = registry.getComponent<game::PlayerStats>(player);
    REQUIRE(updatedPlayerStats.score == 3);
    REQUIRE(updatedPlayerStats.currency == 3);
}

TEST_CASE_METHOD(TestFixture, "DamageSystem converts negative damage to percentage of player max health")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity player = registry.createEntity();
    game::PlayerStats playerStats;
    playerStats.maxHealth = 200.0f;
    playerStats.health = 200.0f;
    registry.addComponent<game::PlayerStats>(player, playerStats);

    const game::Entity projectile = registry.createEntity();
    registry.addComponent<game::Damage>(
        projectile, {.amount = -0.25f,
                     .pushBackForce = 0.0f,
                     .stunChance = 0.0f,
                     .kind = game::DamageKind::Projectile,
                     .params = game::ProjectileDamage{
                         .speed = 0.0f, .maxRange = 100.0f, .distanceTraveled = 0.0f, .maxTargets = 1}});
    registry.addComponent<game::DamageTag>(projectile, {.targets = {player}});

    system.update(registry, 0.016f);

    REQUIRE_FALSE(registry.isEntityAlive(projectile));
    REQUIRE(registry.isEntityAlive(player));

    const auto &updatedPlayerStats = registry.getComponent<game::PlayerStats>(player);
    REQUIRE(updatedPlayerStats.health == Catch::Approx(150.0f));
}

TEST_CASE_METHOD(TestFixture, "DamageSystem converts negative damage to percentage of enemy max health")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity enemy = registry.createEntity();
    game::EnemyStats enemyStats;
    enemyStats.maxHealth = 80.0f;
    enemyStats.health = 80.0f;
    enemyStats.scoreReward = 1;
    registry.addComponent<game::EnemyStats>(enemy, enemyStats);

    const game::Entity projectile = registry.createEntity();
    registry.addComponent<game::Damage>(
        projectile, {.amount = -0.5f,
                     .pushBackForce = 0.0f,
                     .stunChance = 0.0f,
                     .kind = game::DamageKind::Projectile,
                     .params = game::ProjectileDamage{
                         .speed = 0.0f, .maxRange = 100.0f, .distanceTraveled = 0.0f, .maxTargets = 1}});
    registry.addComponent<game::DamageTag>(projectile, {.targets = {enemy}});

    system.update(registry, 0.016f);

    REQUIRE_FALSE(registry.isEntityAlive(projectile));
    REQUIRE(registry.isEntityAlive(enemy));

    const auto &updatedEnemyStats = registry.getComponent<game::EnemyStats>(enemy);
    REQUIRE(updatedEnemyStats.health == Catch::Approx(40.0f));
}

TEST_CASE_METHOD(TestFixture, "DamageSystem rewards player when negative damage kills enemy")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity player = registry.createEntity();
    game::PlayerStats playerStats;
    playerStats.health = 100.0f;
    registry.addComponent<game::PlayerStats>(player, playerStats);

    const game::Entity enemy = registry.createEntity();
    game::EnemyStats enemyStats;
    enemyStats.maxHealth = 80.0f;
    enemyStats.health = 20.0f;
    enemyStats.scoreReward = 7;
    registry.addComponent<game::EnemyStats>(enemy, enemyStats);

    const game::Entity projectile = registry.createEntity();
    registry.addComponent<game::Damage>(
        projectile, {.amount = -0.25f,
                     .pushBackForce = 0.0f,
                     .stunChance = 0.0f,
                     .kind = game::DamageKind::Projectile,
                     .params = game::ProjectileDamage{
                         .speed = 0.0f, .maxRange = 100.0f, .distanceTraveled = 0.0f, .maxTargets = 1}});
    registry.addComponent<game::DamageTag>(projectile, {.targets = {enemy}});

    system.update(registry, 0.016f);

    REQUIRE_FALSE(registry.isEntityAlive(projectile));
    REQUIRE_FALSE(registry.isEntityAlive(enemy));

    const auto &updatedPlayerStats = registry.getComponent<game::PlayerStats>(player);
    REQUIRE(updatedPlayerStats.score == 7);
    REQUIRE(updatedPlayerStats.currency == 7);
}

TEST_CASE_METHOD(TestFixture, "DamageSystem applies negative damage independently per target max health")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity player = registry.createEntity();
    game::PlayerStats playerStats;
    playerStats.maxHealth = 200.0f;
    playerStats.health = 200.0f;
    registry.addComponent<game::PlayerStats>(player, playerStats);

    const game::Entity enemy = registry.createEntity();
    game::EnemyStats enemyStats;
    enemyStats.maxHealth = 80.0f;
    enemyStats.health = 80.0f;
    enemyStats.scoreReward = 1;
    registry.addComponent<game::EnemyStats>(enemy, enemyStats);

    const game::Entity projectile = registry.createEntity();
    registry.addComponent<game::Damage>(
        projectile, {.amount = -0.25f,
                     .pushBackForce = 0.0f,
                     .stunChance = 0.0f,
                     .kind = game::DamageKind::Projectile,
                     .params = game::ProjectileDamage{
                         .speed = 0.0f, .maxRange = 100.0f, .distanceTraveled = 0.0f, .maxTargets = 2}});
    registry.addComponent<game::DamageTag>(projectile, {.targets = {player, enemy}});

    system.update(registry, 0.016f);

    REQUIRE_FALSE(registry.isEntityAlive(projectile));
    REQUIRE(registry.isEntityAlive(player));
    REQUIRE(registry.isEntityAlive(enemy));

    const auto &updatedPlayerStats = registry.getComponent<game::PlayerStats>(player);
    const auto &updatedEnemyStats = registry.getComponent<game::EnemyStats>(enemy);

    REQUIRE(updatedPlayerStats.health == Catch::Approx(150.0f));
    REQUIRE(updatedEnemyStats.health == Catch::Approx(60.0f));
}

TEST_CASE_METHOD(TestFixture, "DamageSystem projectile uses configured speed when velocity component is absent")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity projectile = registry.createEntity();
    registry.addComponent<game::Damage>(
        projectile, {.amount = 5.0f,
                     .pushBackForce = 0.0f,
                     .stunChance = 0.0f,
                     .kind = game::DamageKind::Projectile,
                     .params = game::ProjectileDamage{
                         .speed = 7.0f, .maxRange = 100.0f, .distanceTraveled = 0.0f, .maxTargets = 2}});
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
                     .pushBackForce = 0.0f,
                     .stunChance = 0.0f,
                     .kind = game::DamageKind::Projectile,
                     .params = game::ProjectileDamage{
                         .speed = 0.0f, .maxRange = 100.0f, .distanceTraveled = 0.0f, .maxTargets = 1}});

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
    registry.addComponent<game::Damage>(beam, {.amount = 3.0f,
                                               .pushBackForce = 0.0f,
                                               .stunChance = 0.0f,
                                               .kind = game::DamageKind::Beam,
                                               .params = game::BeamDamage{.length = 80.0f,
                                                                          .width = 12.0f,
                                                                          .activeTimeSec = 0.2f,
                                                                          .elapsedSec = 0.0f,
                                                                          .damageTicks = 1,
                                                                          .elapsedSecSinceLastTick = 0.0f}});
    registry.addComponent<game::DamageTag>(beam, {});

    const game::Entity area = registry.createEntity();
    registry.addComponent<game::Damage>(area, {.amount = 3.0f,
                                               .pushBackForce = 0.0f,
                                               .stunChance = 0.0f,
                                               .kind = game::DamageKind::Area,
                                               .params = game::AreaDamage{.radius = 40.0f,
                                                                          .activeTimeSec = 0.4f,
                                                                          .elapsedSec = 0.0f,
                                                                          .initialHit = 0.0f,
                                                                          .damageTicks = 1,
                                                                          .elapsedSecSinceLastTick = 0.0f}});
    registry.addComponent<game::DamageTag>(area, {});

    system.update(registry, 0.21f);
    REQUIRE_FALSE(registry.isEntityAlive(beam));
    REQUIRE(registry.isEntityAlive(area));

    system.update(registry, 0.2f);
    REQUIRE_FALSE(registry.isEntityAlive(area));
}

TEST_CASE_METHOD(TestFixture, "DamageSystem projectile is removed after reaching max targets")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity projectile = registry.createEntity();
    registry.addComponent<game::Damage>(
        projectile, {.amount = 4.0f,
                     .pushBackForce = 0.0f,
                     .stunChance = 0.0f,
                     .kind = game::DamageKind::Projectile,
                     .params = game::ProjectileDamage{
                         .speed = 0.0f, .maxRange = 100.0f, .distanceTraveled = 0.0f, .maxTargets = 1}});

    const game::Entity enemyA = registry.createEntity();
    game::EnemyStats enemyAStats;
    enemyAStats.health = 10.0f;
    enemyAStats.scoreReward = 1;
    registry.addComponent<game::EnemyStats>(enemyA, enemyAStats);

    const game::Entity enemyB = registry.createEntity();
    game::EnemyStats enemyBStats;
    enemyBStats.health = 10.0f;
    enemyBStats.scoreReward = 1;
    registry.addComponent<game::EnemyStats>(enemyB, enemyBStats);

    registry.addComponent<game::DamageTag>(projectile, {.targets = {enemyA, enemyB}});

    system.update(registry, 0.016f);

    REQUIRE_FALSE(registry.isEntityAlive(projectile));
    REQUIRE(registry.isEntityAlive(enemyA));
    REQUIRE(registry.isEntityAlive(enemyB));
}

TEST_CASE_METHOD(TestFixture, "DamageSystem removes dead tagged targets from projectile cleanup")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity projectile = registry.createEntity();
    registry.addComponent<game::Damage>(
        projectile, {.amount = 1.0f,
                     .pushBackForce = 0.0f,
                     .stunChance = 0.0f,
                     .kind = game::DamageKind::Projectile,
                     .params = game::ProjectileDamage{
                         .speed = 0.0f, .maxRange = 100.0f, .distanceTraveled = 0.0f, .maxTargets = 3}});

    const game::Entity aliveTarget = registry.createEntity();
    game::EnemyStats aliveStats;
    aliveStats.health = 10.0f;
    aliveStats.scoreReward = 1;
    registry.addComponent<game::EnemyStats>(aliveTarget, aliveStats);

    const game::Entity deadTarget = registry.createEntity();
    game::EnemyStats deadStats;
    deadStats.health = 10.0f;
    deadStats.scoreReward = 1;
    registry.addComponent<game::EnemyStats>(deadTarget, deadStats);
    registry.destroyEntity(deadTarget);

    registry.addComponent<game::DamageTag>(projectile, {.targets = {deadTarget, aliveTarget}});

    system.update(registry, 0.016f);

    REQUIRE(registry.isEntityAlive(projectile));
    const auto &tag = registry.getComponent<game::DamageTag>(projectile);
    REQUIRE(tag.targets.contains(aliveTarget));
    REQUIRE_FALSE(tag.targets.contains(deadTarget));
}

TEST_CASE_METHOD(TestFixture, "DamageSystem beam clears targets and periodically resets hit cache")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity player = registry.createEntity();
    game::PlayerStats playerStats;
    playerStats.health = 100.0f;
    registry.addComponent<game::PlayerStats>(player, playerStats);

    const game::Entity beam = registry.createEntity();
    registry.addComponent<game::Damage>(beam, {.amount = 12.0f,
                                               .pushBackForce = 0.0f,
                                               .stunChance = 0.0f,
                                               .kind = game::DamageKind::Beam,
                                               .params = game::BeamDamage{.length = 80.0f,
                                                                          .width = 12.0f,
                                                                          .activeTimeSec = 2.0f,
                                                                          .elapsedSec = 0.0f,
                                                                          .damageTicks = 2,
                                                                          .elapsedSecSinceLastTick = 0.0f}});
    registry.addComponent<game::DamageTag>(beam, {.targets = {player}, .targetsHit = {player}});

    system.update(registry, 0.2f);

    auto &beamTag = registry.getComponent<game::DamageTag>(beam);
    REQUIRE(beamTag.targets.empty());
    REQUIRE(beamTag.targetsHit.empty());

    beamTag.targetsHit.insert(player);
    system.update(registry, 0.1f);

    REQUIRE(beamTag.targets.empty());
    REQUIRE(beamTag.targetsHit.contains(player));
}

TEST_CASE_METHOD(TestFixture, "DamageSystem area damage applies initial and tick damage and clears targets")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity player = registry.createEntity();
    game::PlayerStats playerStats;
    playerStats.health = 100.0f;
    registry.addComponent<game::PlayerStats>(player, playerStats);

    const game::Entity area = registry.createEntity();
    registry.addComponent<game::Damage>(area, {.amount = 10.0f,
                                               .pushBackForce = 0.0f,
                                               .stunChance = 0.0f,
                                               .kind = game::DamageKind::Area,
                                               .params = game::AreaDamage{.radius = 40.0f,
                                                                          .activeTimeSec = 1.0f,
                                                                          .elapsedSec = 0.0f,
                                                                          .initialHit = 0.4f,
                                                                          .damageTicks = 2,
                                                                          .elapsedSecSinceLastTick = 0.0f}});
    registry.addComponent<game::DamageTag>(area, {.targets = {player}});

    system.update(registry, 0.05f);

    auto &updatedPlayer = registry.getComponent<game::PlayerStats>(player);
    REQUIRE(updatedPlayer.health == Catch::Approx(96.0f));
    auto &areaTag = registry.getComponent<game::DamageTag>(area);
    REQUIRE(areaTag.targets.empty());
    REQUIRE(areaTag.targetsHit.contains(player));

    areaTag.targets.insert(player);
    system.update(registry, 0.05f);
    REQUIRE(updatedPlayer.health == Catch::Approx(96.0f));

    areaTag.targets.insert(player);
    system.update(registry, 0.5f);

    REQUIRE(updatedPlayer.health == Catch::Approx(93.0f));
    REQUIRE(areaTag.targets.empty());
    REQUIRE(areaTag.targetsHit.contains(player));
}

TEST_CASE_METHOD(TestFixture, "DamageSystem area negative damage uses percentage instead of initial and tick scaling")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity player = registry.createEntity();
    game::PlayerStats playerStats;
    playerStats.maxHealth = 200.0f;
    playerStats.health = 200.0f;
    registry.addComponent<game::PlayerStats>(player, playerStats);

    const game::Entity area = registry.createEntity();
    registry.addComponent<game::Damage>(area, {.amount = -0.25f,
                                               .pushBackForce = 0.0f,
                                               .stunChance = 0.0f,
                                               .kind = game::DamageKind::Area,
                                               .params = game::AreaDamage{.radius = 40.0f,
                                                                          .activeTimeSec = 1.0f,
                                                                          .elapsedSec = 0.0f,
                                                                          .initialHit = 0.4f,
                                                                          .damageTicks = 2,
                                                                          .elapsedSecSinceLastTick = 0.0f}});
    registry.addComponent<game::DamageTag>(area, {.targets = {player}});

    system.update(registry, 0.05f);

    auto &updatedPlayer = registry.getComponent<game::PlayerStats>(player);
    auto &areaTag = registry.getComponent<game::DamageTag>(area);

    REQUIRE(updatedPlayer.health == Catch::Approx(150.0f));
    REQUIRE(areaTag.targets.empty());
    REQUIRE(areaTag.targetsHit.contains(player));

    areaTag.targets.insert(player);
    system.update(registry, 0.05f);

    REQUIRE(updatedPlayer.health == Catch::Approx(150.0f));
    REQUIRE(areaTag.targets.empty());
    REQUIRE(areaTag.targetsHit.contains(player));

    areaTag.targets.insert(player);
    system.update(registry, 0.5f);

    REQUIRE(updatedPlayer.health == Catch::Approx(100.0f));
    REQUIRE(areaTag.targets.empty());
    REQUIRE(areaTag.targetsHit.contains(player));
}

TEST_CASE_METHOD(TestFixture, "DamageSystem area telegraph delays non-initial damage application")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity player = registry.createEntity();
    game::PlayerStats playerStats;
    playerStats.health = 100.0f;
    registry.addComponent<game::PlayerStats>(player, playerStats);

    const game::Entity area = registry.createEntity();
    registry.addComponent<game::Damage>(area, {.amount = 12.0f,
                                               .pushBackForce = 0.0f,
                                               .stunChance = 0.0f,
                                               .kind = game::DamageKind::Area,
                                               .params = game::AreaDamage{.radius = 40.0f,
                                                                          .activeTimeSec = 1.0f,
                                                                          .elapsedSec = 0.0f,
                                                                          .telegraphTimeSec = 0.5f,
                                                                          .initialHit = 0.0f,
                                                                          .damageTicks = 1,
                                                                          .elapsedSecSinceLastTick = 0.0f}});
    registry.addComponent<game::DamageTag>(area, {.targets = {player}});

    system.update(registry, 0.25f);

    auto &updatedPlayer = registry.getComponent<game::PlayerStats>(player);
    auto &areaTag = registry.getComponent<game::DamageTag>(area);

    REQUIRE(updatedPlayer.health == Catch::Approx(100.0f));
    REQUIRE(areaTag.targetsHit.empty());

    areaTag.targets.insert(player);
    system.update(registry, 0.25f);

    REQUIRE(updatedPlayer.health == Catch::Approx(100.0f));
    REQUIRE(areaTag.targetsHit.empty());

    areaTag.targets.insert(player);
    system.update(registry, 0.01f);

    REQUIRE(updatedPlayer.health == Catch::Approx(88.0f));
    REQUIRE(areaTag.targetsHit.contains(player));
}

TEST_CASE_METHOD(TestFixture, "DamageSystem sets player animation to Hit when damaged")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity player = registry.createEntity();
    game::PlayerStats playerStats;
    playerStats.health = 100.0f;
    registry.addComponent<game::PlayerStats>(player, playerStats);
    registry.addComponent<game::Animation>(
        player, {.state = game::AnimationState::Attack, .direction = game::AnimationDirection::Left});

    const game::Entity projectile = registry.createEntity();
    registry.addComponent<game::Damage>(
        projectile, {.amount = 10.0f,
                     .pushBackForce = 0.0f,
                     .stunChance = 0.0f,
                     .kind = game::DamageKind::Projectile,
                     .params = game::ProjectileDamage{
                         .speed = 0.0f, .maxRange = 100.0f, .distanceTraveled = 0.0f, .maxTargets = 1}});
    registry.addComponent<game::DamageTag>(projectile, {.targets = {player}});

    system.update(registry, 0.016f);

    const game::Animation &animation = registry.getComponent<game::Animation>(player);
    REQUIRE(animation.state == game::AnimationState::Hit);
    REQUIRE(animation.direction == game::AnimationDirection::Left);
    REQUIRE(animation.currentFrame == 0);
    REQUIRE(animation.frameTimer == Catch::Approx(0.0f));
    REQUIRE(animation.stateTimeRemaining == Catch::Approx(0.16f));
}

TEST_CASE_METHOD(TestFixture, "DamageSystem sets enemy animation to Hit when damaged")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity enemy = registry.createEntity();
    game::EnemyStats enemyStats;
    enemyStats.health = 100.0f;
    enemyStats.scoreReward = 1;
    registry.addComponent<game::EnemyStats>(enemy, enemyStats);
    registry.addComponent<game::Animation>(
        enemy, {.state = game::AnimationState::Walk, .direction = game::AnimationDirection::Right});

    const game::Entity projectile = registry.createEntity();
    registry.addComponent<game::Damage>(
        projectile, {.amount = 10.0f,
                     .pushBackForce = 0.0f,
                     .stunChance = 0.0f,
                     .kind = game::DamageKind::Projectile,
                     .params = game::ProjectileDamage{
                         .speed = 0.0f, .maxRange = 100.0f, .distanceTraveled = 0.0f, .maxTargets = 1}});
    registry.addComponent<game::DamageTag>(projectile, {.targets = {enemy}});

    system.update(registry, 0.016f);

    const game::Animation &animation = registry.getComponent<game::Animation>(enemy);
    REQUIRE(animation.state == game::AnimationState::Hit);
    REQUIRE(animation.direction == game::AnimationDirection::Right);
    REQUIRE(animation.currentFrame == 0);
    REQUIRE(animation.frameTimer == Catch::Approx(0.0f));
    REQUIRE(animation.stateTimeRemaining == Catch::Approx(0.16f));
}

TEST_CASE_METHOD(TestFixture, "DamageSystem keeps unicorn damage entity while intersecting map")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity map = registry.createEntity();
    registry.addComponent<game::MapTag>(map, {});
    registry.addComponent<game::Position>(map, {.p = {0.0f, 0.0f}});
    registry.addComponent<game::HitBox>(map, {.offset = {0.0f, 0.0f}, .size = {100.0f, 100.0f}});

    const game::Entity unicorn = registry.createEntity();
    registry.addComponent<game::Position>(unicorn, {.p = {20.0f, 20.0f}});
    registry.addComponent<game::HitBox>(unicorn, {.offset = {0.0f, 0.0f}, .size = {16.0f, 16.0f}});
    registry.addComponent<game::Damage>(unicorn, {.amount = 15.0f,
                                                  .pushBackForce = 0.0f,
                                                  .stunChance = 0.0f,
                                                  .kind = game::DamageKind::Unicorn,
                                                  .params = game::UnicornDamage{}});
    registry.addComponent<game::DamageTag>(unicorn, {});

    system.update(registry, 0.016f);

    REQUIRE(registry.isEntityAlive(unicorn));
}

TEST_CASE_METHOD(TestFixture, "DamageSystem destroys unicorn damage entity after leaving map")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity map = registry.createEntity();
    registry.addComponent<game::MapTag>(map, {});
    registry.addComponent<game::Position>(map, {.p = {0.0f, 0.0f}});
    registry.addComponent<game::HitBox>(map, {.offset = {0.0f, 0.0f}, .size = {100.0f, 100.0f}});

    const game::Entity unicorn = registry.createEntity();
    registry.addComponent<game::Position>(unicorn, {.p = {150.0f, 20.0f}});
    registry.addComponent<game::HitBox>(unicorn, {.offset = {0.0f, 0.0f}, .size = {16.0f, 16.0f}});
    registry.addComponent<game::Damage>(unicorn, {.amount = 15.0f,
                                                  .pushBackForce = 0.0f,
                                                  .stunChance = 0.0f,
                                                  .kind = game::DamageKind::Unicorn,
                                                  .params = game::UnicornDamage{}});
    registry.addComponent<game::DamageTag>(unicorn, {});

    system.update(registry, 0.016f);

    REQUIRE_FALSE(registry.isEntityAlive(unicorn));
}

TEST_CASE_METHOD(TestFixture, "DamageSystem keeps unicorn damage entity when map entity is missing")
{
    game::Registry registry;
    game::DamageSystem system;

    const game::Entity unicorn = registry.createEntity();
    registry.addComponent<game::Position>(unicorn, {.p = {150.0f, 20.0f}});
    registry.addComponent<game::HitBox>(unicorn, {.offset = {0.0f, 0.0f}, .size = {16.0f, 16.0f}});
    registry.addComponent<game::Damage>(unicorn, {.amount = 15.0f,
                                                  .pushBackForce = 0.0f,
                                                  .stunChance = 0.0f,
                                                  .kind = game::DamageKind::Unicorn,
                                                  .params = game::UnicornDamage{}});
    registry.addComponent<game::DamageTag>(unicorn, {});

    system.update(registry, 0.016f);

    REQUIRE(registry.isEntityAlive(unicorn));
}