#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/health_bar_state.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/health_bar_system.hpp"
#include "shared/test_fixture.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

// ── HealthBarState component ──────────────────────────────────────────────────

TEST_CASE("HealthBarState default-constructed has uninitialized previousHealth")
{
    game::HealthBarState bar;
    REQUIRE(bar.previousHealth < 0.0f);
    REQUIRE(bar.initialRedBarNorm == 0.0f);
    REQUIRE(bar.redBarTimer == 0.0f);
}

TEST_CASE("HealthBarState redFlashDuration is positive")
{
    REQUIRE(game::HealthBarState::redFlashDuration > 0.0f);
}

// ── HealthBarSystem ───────────────────────────────────────────────────────────

TEST_CASE_METHOD(TestFixture, "HealthBarSystem initialises previousHealth on first update")
{
    game::Registry registry;
    game::HealthBarSystem system;

    game::Entity entity = registry.createEntity();
    game::PlayerStats stats;
    stats.maxHealth = 100.0f;
    stats.health = 80.0f;
    registry.addComponent<game::PlayerTag>(entity, {});
    registry.addComponent<game::PlayerStats>(entity, stats);
    registry.addComponent<game::HealthBarState>(entity, {});

    system.update(registry, 0.016f);

    const auto &bar = registry.getComponent<game::HealthBarState>(entity);
    REQUIRE(bar.previousHealth == 80.0f);
    REQUIRE(bar.redBarTimer == 0.0f);
    REQUIRE(bar.initialRedBarNorm == 0.0f);
}

TEST_CASE_METHOD(TestFixture, "HealthBarSystem sets redBarTimer when player takes damage")
{
    game::Registry registry;
    game::HealthBarSystem system;

    game::Entity entity = registry.createEntity();
    game::PlayerStats stats;
    stats.maxHealth = 100.0f;
    stats.health = 100.0f;
    registry.addComponent<game::PlayerTag>(entity, {});
    registry.addComponent<game::PlayerStats>(entity, stats);
    registry.addComponent<game::HealthBarState>(entity, {});

    // First update — initialises previousHealth
    system.update(registry, 0.016f);

    // Simulate taking 20 damage
    registry.getComponent<game::PlayerStats>(entity).health = 80.0f;
    system.update(registry, 0.016f);

    const auto &bar = registry.getComponent<game::HealthBarState>(entity);
    REQUIRE(bar.redBarTimer == game::HealthBarState::redFlashDuration);
    REQUIRE(bar.initialRedBarNorm == Catch::Approx(0.2f)); // 20/100
    REQUIRE(bar.previousHealth == 80.0f);
}

TEST_CASE_METHOD(TestFixture, "HealthBarSystem sets redBarTimer for enemy when it takes damage")
{
    game::Registry registry;
    game::HealthBarSystem system;

    game::Entity entity = registry.createEntity();
    game::EnemyStats stats;
    stats.maxHealth = 50.0f;
    stats.health = 50.0f;
    registry.addComponent<game::EnemyTag>(entity, {});
    registry.addComponent<game::EnemyStats>(entity, stats);
    registry.addComponent<game::HealthBarState>(entity, {});

    system.update(registry, 0.016f);

    registry.getComponent<game::EnemyStats>(entity).health = 25.0f;
    system.update(registry, 0.016f);

    const auto &bar = registry.getComponent<game::HealthBarState>(entity);
    REQUIRE(bar.redBarTimer == game::HealthBarState::redFlashDuration);
    REQUIRE(bar.initialRedBarNorm == 0.5f); // 25/50
}

TEST_CASE_METHOD(TestFixture, "HealthBarSystem decrements redBarTimer each update")
{
    game::Registry registry;
    game::HealthBarSystem system;

    game::Entity entity = registry.createEntity();
    game::PlayerStats stats;
    stats.maxHealth = 100.0f;
    stats.health = 100.0f;
    registry.addComponent<game::PlayerTag>(entity, {});
    registry.addComponent<game::PlayerStats>(entity, stats);
    registry.addComponent<game::HealthBarState>(entity, {});

    system.update(registry, 0.016f);

    registry.getComponent<game::PlayerStats>(entity).health = 80.0f;
    system.update(registry, 0.016f);

    // Timer should now be counting down
    const float dt = 0.1f;
    system.update(registry, dt);

    const auto &bar = registry.getComponent<game::HealthBarState>(entity);
    REQUIRE(bar.redBarTimer == game::HealthBarState::redFlashDuration - dt);
}

TEST_CASE_METHOD(TestFixture, "HealthBarSystem clears red bar when timer reaches zero")
{
    game::Registry registry;
    game::HealthBarSystem system;

    game::Entity entity = registry.createEntity();
    game::PlayerStats stats;
    stats.maxHealth = 100.0f;
    stats.health = 100.0f;
    registry.addComponent<game::PlayerTag>(entity, {});
    registry.addComponent<game::PlayerStats>(entity, stats);
    registry.addComponent<game::HealthBarState>(entity, {});

    system.update(registry, 0.016f);

    registry.getComponent<game::PlayerStats>(entity).health = 80.0f;
    system.update(registry, 0.016f);

    // Advance past the full duration in one step
    system.update(registry, game::HealthBarState::redFlashDuration + 1.0f);

    const auto &bar = registry.getComponent<game::HealthBarState>(entity);
    REQUIRE(bar.redBarTimer == 0.0f);
    REQUIRE(bar.initialRedBarNorm == 0.0f);
}

TEST_CASE_METHOD(TestFixture, "HealthBarSystem does not set timer when health is unchanged")
{
    game::Registry registry;
    game::HealthBarSystem system;

    game::Entity entity = registry.createEntity();
    game::PlayerStats stats;
    stats.maxHealth = 100.0f;
    stats.health = 75.0f;
    registry.addComponent<game::PlayerTag>(entity, {});
    registry.addComponent<game::PlayerStats>(entity, stats);
    registry.addComponent<game::HealthBarState>(entity, {});

    system.update(registry, 0.016f); // init
    system.update(registry, 0.016f); // no damage

    const auto &bar = registry.getComponent<game::HealthBarState>(entity);
    REQUIRE(bar.redBarTimer == 0.0f);
    REQUIRE(bar.initialRedBarNorm == 0.0f);
}

TEST_CASE_METHOD(TestFixture, "HealthBarSystem is no-op when no entities have HealthBarState")
{
    game::Registry registry;
    game::HealthBarSystem system;

    game::Entity entity = registry.createEntity();
    game::PlayerStats stats;
    stats.maxHealth = 100.0f;
    stats.health = 100.0f;
    registry.addComponent<game::PlayerTag>(entity, {});
    registry.addComponent<game::PlayerStats>(entity, stats);
    // No HealthBarState added

    REQUIRE_NOTHROW(system.update(registry, 0.016f));
}

TEST_CASE_METHOD(TestFixture, "HealthBarSystem accumulates red bar norm on rapid successive hits")
{
    game::Registry registry;
    game::HealthBarSystem system;

    game::Entity entity = registry.createEntity();
    game::PlayerStats stats;
    stats.maxHealth = 100.0f;
    stats.health = 100.0f;
    registry.addComponent<game::PlayerTag>(entity, {});
    registry.addComponent<game::PlayerStats>(entity, stats);
    registry.addComponent<game::HealthBarState>(entity, {});

    system.update(registry, 0.016f); // init

    // First hit: -10 hp
    registry.getComponent<game::PlayerStats>(entity).health = 90.0f;
    system.update(registry, 0.016f);

    // Second hit before timer expires: -10 hp more
    registry.getComponent<game::PlayerStats>(entity).health = 80.0f;
    system.update(registry, 0.016f);

    const auto &bar = registry.getComponent<game::HealthBarState>(entity);
    // Total missing = 20%, accumulated red should not exceed that
    REQUIRE(bar.initialRedBarNorm <= 0.2f + 1e-5f);
    REQUIRE(bar.initialRedBarNorm > 0.0f);
    REQUIRE(bar.redBarTimer == game::HealthBarState::redFlashDuration);
}
