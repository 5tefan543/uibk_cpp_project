#include "config/game_config.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/boss_phase.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/distance_to_player.hpp"
#include "game/ecs/components/enemy_attack_tag.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/boss_attack_system.hpp"
#include "game/ecs/systems/player_distance_system.hpp"
#include "shared/test_fixture.hpp"
#include "view/sprite.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>

namespace {

config::GameConfig makeBossAttackTestConfig()
{
    config::GameConfig config{};

    config.fallbackSprite.texture.path = "fallback.png";
    config.fallbackSprite.texture.size = {16.0f, 16.0f};
    config.fallbackSprite.hitBox.offset = {0.0f, 0.0f};
    config.fallbackSprite.hitBox.size = {16.0f, 16.0f};

    config.enemyClasses.boss.attack.amount = 5.0f;
    config.enemyClasses.boss.attack.pushBackForce = 0.0f;
    config.enemyClasses.boss.attack.stunChance = 0.0f;
    config.enemyClasses.boss.attack.projectile.velocityScale = 1.0f;

    config::SpriteConfig projectileSpriteConfig{};
    projectileSpriteConfig.texture.path = "boss_projectile.png";
    projectileSpriteConfig.texture.size = {128.0f, 128.0f};
    projectileSpriteConfig.hitBox.offset = {0.0f, 0.0f};
    projectileSpriteConfig.hitBox.size = {128.0f, 128.0f};
    auto &projectileIdle =
        config.enemyClasses.boss.attack.projectile.animations.stateToStateConfig[game::AnimationState::Idle];
    projectileIdle.directionToFrames[game::AnimationDirection::None] = {projectileSpriteConfig};

    config.enemyClasses.boss.attack.area.radius = 80.0f;
    config.enemyClasses.boss.attack.area.activeTimeSec = 0.8f;
    config.enemyClasses.boss.attack.area.telegraphTimeSec = 0.5f;
    config.enemyClasses.boss.attack.area.initialHit = 0.25f;
    config.enemyClasses.boss.attack.area.damageTicks = 2;

    config::SpriteConfig lightningSpriteConfig{};
    lightningSpriteConfig.texture.path = "boss_lightning_1.png";
    lightningSpriteConfig.texture.size = {128.0f, 128.0f};
    lightningSpriteConfig.hitBox.offset = {0.0f, 0.0f};
    lightningSpriteConfig.hitBox.size = {128.0f, 128.0f};
    auto &lightningIdle =
        config.enemyClasses.boss.attack.area.animations.stateToStateConfig[game::AnimationState::Idle];
    lightningIdle.directionToFrames[game::AnimationDirection::None] = {lightningSpriteConfig};

    config::SpriteConfig bossSpriteFrameOne{};
    bossSpriteFrameOne.texture.path = "boss_1.png";
    bossSpriteFrameOne.texture.size = {128.0f, 128.0f};
    bossSpriteFrameOne.hitBox.offset = {0.0f, 0.0f};
    bossSpriteFrameOne.hitBox.size = {128.0f, 128.0f};

    config::SpriteConfig bossSpriteFrameTwo = bossSpriteFrameOne;
    bossSpriteFrameTwo.texture.path = "boss_2.png";

    config::SpriteConfig bossHitFrameOne = bossSpriteFrameOne;
    bossHitFrameOne.texture.path = "boss_hit_1.png";

    config::SpriteConfig bossHitFrameTwo = bossSpriteFrameOne;
    bossHitFrameTwo.texture.path = "boss_hit_2.png";

    for (const auto state : {game::AnimationState::Idle, game::AnimationState::Walk, game::AnimationState::Attack}) {
        auto &animationState = config.enemyClasses.boss.animations.stateToStateConfig[state];
        animationState.directionToFrames[game::AnimationDirection::Left] = {bossSpriteFrameOne, bossSpriteFrameTwo};
        animationState.directionToFrames[game::AnimationDirection::Right] = {bossSpriteFrameOne, bossSpriteFrameTwo};
    }

    auto &hitState = config.enemyClasses.boss.animations.stateToStateConfig[game::AnimationState::Hit];
    hitState.directionToFrames[game::AnimationDirection::Left] = {bossHitFrameOne, bossHitFrameTwo};
    hitState.directionToFrames[game::AnimationDirection::Right] = {bossHitFrameOne, bossHitFrameTwo};

    return config;
}

game::Entity addMap(game::Registry &registry)
{
    const game::Entity map = registry.createEntity();
    registry.addComponent<game::MapTag>(map, {});
    registry.addComponent<game::Position>(map, {{0.0f, 0.0f}});
    registry.addComponent<view::Sprite>(map, {{{0.0f, 0.0f}, {2000.0f, 2000.0f}}, "map.png"});
    return map;
}

game::Entity addPlayer(game::Registry &registry, float x, float y)
{
    const game::Entity player = registry.createEntity();
    registry.addComponent<game::PlayerTag>(player, {});
    registry.addComponent<game::Position>(player, {{x, y}});
    return player;
}

game::Entity addBoss(game::Registry &registry, float x, float y)
{
    const game::Entity boss = registry.createEntity();

    game::EnemyStats stats{};
    stats.enemyType = game::EnemyType::Boss;
    stats.maxHealth = 100.0f;
    stats.health = 100.0f;
    stats.attackSpeed = 1.0f;
    stats.attackRange = 300.0f;
    stats.speedOfAttack = 400.0f;

    registry.addComponent<game::EnemyTag>(boss, {});
    registry.addComponent<game::EnemyStats>(boss, stats);
    registry.addComponent<game::Position>(boss, {{x, y}});
    registry.addComponent<view::Sprite>(boss, {{{x, y}, {256.0f, 256.0f}}, "boss.png"});
    registry.addComponent<game::Animation>(boss, {});

    return boss;
}

std::vector<game::Entity> getBossProjectiles(game::Registry &registry)
{
    std::vector<game::Entity> projectiles;
    for (const game::Entity entity : registry.view<game::EnemyAttackTag, game::Damage, game::Velocity>()) {
        const auto &tag = registry.getComponent<game::EnemyAttackTag>(entity);
        const auto &damage = registry.getComponent<game::Damage>(entity);
        if (tag.enemyType == game::EnemyType::Boss && damage.kind == game::DamageKind::Projectile) {
            projectiles.push_back(entity);
        }
    }
    return projectiles;
}

std::vector<game::Entity> getBossLightning(game::Registry &registry)
{
    std::vector<game::Entity> lightning;
    for (const game::Entity entity : registry.view<game::EnemyAttackTag, game::Damage>()) {
        const auto &tag = registry.getComponent<game::EnemyAttackTag>(entity);
        const auto &damage = registry.getComponent<game::Damage>(entity);
        if (tag.enemyType == game::EnemyType::Boss && damage.kind == game::DamageKind::Area) {
            lightning.push_back(entity);
        }
    }
    return lightning;
}

} // namespace

TEST_CASE_METHOD(TestFixture, "BossAttackSystem switches to phase 2 at or below 50 percent health")
{
    game::Registry registry;
    game::PlayerDistanceSystem distanceSystem;
    game::BossAttackSystem bossSystem;
    const config::GameConfig config = makeBossAttackTestConfig();

    addMap(registry);
    addPlayer(registry, 200.0f, 200.0f);
    const game::Entity boss = addBoss(registry, 100.0f, 100.0f);

    auto &stats = registry.getComponent<game::EnemyStats>(boss);
    stats.health = 50.0f;

    distanceSystem.update(registry);
    bossSystem.update(registry, config, 0.0f);

    REQUIRE(registry.hasComponent<game::BossPhase>(boss));
    const auto &phase = registry.getComponent<game::BossPhase>(boss);
    REQUIRE(phase.phase == game::BossPhaseState::Phase2);
}

TEST_CASE_METHOD(TestFixture, "BossAttackSystem spawns radial projectile burst with outward velocity")
{
    game::Registry registry;
    game::PlayerDistanceSystem distanceSystem;
    game::BossAttackSystem bossSystem;
    const config::GameConfig config = makeBossAttackTestConfig();

    addMap(registry);
    addPlayer(registry, 250.0f, 250.0f);
    const game::Entity boss = addBoss(registry, 100.0f, 100.0f);

    distanceSystem.update(registry);
    bossSystem.update(registry, config, 0.0f);

    const auto projectiles = getBossProjectiles(registry);
    REQUIRE(projectiles.size() == 12);

    const auto &animation = registry.getComponent<game::Animation>(boss);
    REQUIRE(animation.state == game::AnimationState::Attack);
    REQUIRE(animation.stateTimeRemaining > 0.0f);

    const auto &bossPosition = registry.getComponent<game::Position>(boss).p;
    const auto &bossSprite = registry.getComponent<view::Sprite>(boss).rect;
    const auto bossCenter = bossPosition + (bossSprite.size / 2.0f);

    for (const game::Entity projectile : projectiles) {
        const auto &position = registry.getComponent<game::Position>(projectile).p;
        const auto &spriteComponent = registry.getComponent<view::Sprite>(projectile);
        const auto &sprite = spriteComponent.rect;
        const auto projectileCenter = position + (sprite.size / 2.0f);
        const auto direction = projectileCenter - bossCenter;
        const auto &velocity = registry.getComponent<game::Velocity>(projectile).v;

        REQUIRE(spriteComponent.imagePath == "boss_projectile.png");
        const float dot = (direction.x * velocity.x) + (direction.y * velocity.y);
        REQUIRE(dot > 0.0f);
    }
}

TEST_CASE_METHOD(TestFixture, "BossAttackSystem phase 2 lightning triggers regardless of player range")
{
    game::Registry registry;
    game::PlayerDistanceSystem distanceSystem;
    game::BossAttackSystem bossSystem;
    const config::GameConfig config = makeBossAttackTestConfig();

    addMap(registry);
    const geometry::Vec2<float> playerPosition{1000.0f, 1000.0f};
    addPlayer(registry, playerPosition.x, playerPosition.y);
    const game::Entity boss = addBoss(registry, 0.0f, 0.0f);

    auto &stats = registry.getComponent<game::EnemyStats>(boss);
    stats.health = 40.0f;
    stats.attackRange = 1.0f;

    distanceSystem.update(registry);
    bossSystem.update(registry, config, 0.0f);

    const auto lightning = getBossLightning(registry);
    REQUIRE_FALSE(lightning.empty());

    const auto players = registry.view<game::PlayerTag, game::Position>();
    REQUIRE(players.size() == 1);
    const auto &storedPlayerPosition = registry.getComponent<game::Position>(players.front()).p;
    CAPTURE(storedPlayerPosition.x, storedPlayerPosition.y);

    for (const game::Entity lightningEntity : lightning) {
        REQUIRE(registry.hasComponent<game::Animation>(lightningEntity));

        const auto &sprite = registry.getComponent<view::Sprite>(lightningEntity);
        const auto &position = registry.getComponent<game::Position>(lightningEntity).p;
        const auto lightningCenter = position + (sprite.rect.size / 2.0f);
        const auto offset = lightningCenter - playerPosition;
        CAPTURE(position.x, position.y, lightningCenter.x, lightningCenter.y, offset.x, offset.y);
        REQUIRE(sprite.imagePath == "boss_lightning_1.png");
        REQUIRE(offset.length() <= Catch::Approx(1400.0f));
    }
}

TEST_CASE_METHOD(TestFixture, "BossAttackSystem cooldown prevents immediate retrigger")
{
    game::Registry registry;
    game::PlayerDistanceSystem distanceSystem;
    game::BossAttackSystem bossSystem;
    const config::GameConfig config = makeBossAttackTestConfig();

    addMap(registry);
    addPlayer(registry, 250.0f, 250.0f);
    addBoss(registry, 100.0f, 100.0f);

    distanceSystem.update(registry);
    bossSystem.update(registry, config, 0.0f);
    const std::size_t firstCount = getBossProjectiles(registry).size();
    REQUIRE(firstCount == 12);

    distanceSystem.update(registry);
    bossSystem.update(registry, config, 0.1f);
    const std::size_t secondCount = getBossProjectiles(registry).size();

    REQUIRE(secondCount == firstCount);
}
