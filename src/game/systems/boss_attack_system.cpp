#include "game/ecs/systems/boss_attack_system.hpp"
#include "config/animation_config_helper.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/boss_phase.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/distance_to_player.hpp"
#include "game/ecs/components/enemy_attack_cooldown.hpp"
#include "game/ecs/components/enemy_attack_tag.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/sound.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "view/sprite.hpp"
#include <algorithm>
#include <array>
#include <cmath>

namespace game {

namespace {

constexpr float pi = 3.14159265358979323846f;
constexpr int radialProjectileCount = 12;
constexpr int phaseTwoLightningStrikeCount = 3;
constexpr float defaultProjectileSpeed = 250.0f;
constexpr float defaultProjectileRange = 800.0f;
constexpr float minProjectileRangeTrigger = 512.0f;

Animation createAttackAnimation(const config::AnimationFrame &frame)
{
    Animation animation{.state = AnimationState::Idle, .direction = AnimationDirection::None};
    const float animationDuration = static_cast<float>(frame.totalFrames) * frame.frameDuration;
    startTimedAnimation(animation, AnimationState::Idle, AnimationDirection::None, animationDuration);
    return animation;
}

float applyBossAttackAnimation(Registry &registry, const config::GameConfig &config, const Entity bossEntity,
                               const EnemyType enemyType, AnimationDirection direction)
{
    if (direction == AnimationDirection::None) {
        direction = AnimationDirection::Right;
    }

    Animation &bossAnimation = registry.getComponent<Animation>(bossEntity);
    const config::AnimationFrame firstBossFrame =
        config::AnimationConfigHelper::getEnemyAnimationFrame(config, enemyType, AnimationState::Attack, direction, 0);
    const float animationDuration = static_cast<float>(firstBossFrame.totalFrames) * firstBossFrame.frameDuration;
    startTimedAnimation(bossAnimation, AnimationState::Attack, direction, animationDuration);

    return animationDuration;
}

geometry::Vec2<float> clampSpritePositionToMap(const geometry::Vec2<float> &position,
                                               const geometry::Vec2<float> &spriteSize,
                                               const geometry::Vec2<float> &mapPosition,
                                               const geometry::Vec2<float> &mapSize)
{
    const float minX = mapPosition.x;
    const float minY = mapPosition.y;
    const float maxX = std::max(minX, mapPosition.x + mapSize.x - spriteSize.x);
    const float maxY = std::max(minY, mapPosition.y + mapSize.y - spriteSize.y);

    return {std::clamp(position.x, minX, maxX), std::clamp(position.y, minY, maxY)};
}

} // namespace

BossAttackSystem::BossAttackSystem() : randomEngine_(std::random_device{}()) {}

void BossAttackSystem::updateCoolDown(Registry &registry, Entity enemyEntity, float dtSec)
{
    if (!registry.hasComponent<EnemyAttackCooldown>(enemyEntity)) {
        return;
    }

    EnemyAttackCooldown &cooldown = registry.getComponent<EnemyAttackCooldown>(enemyEntity);
    cooldown.remainingSec -= dtSec;
    if (cooldown.remainingSec <= 0.0f) {
        registry.removeComponent<EnemyAttackCooldown>(enemyEntity);
    }
}

void BossAttackSystem::ensurePhaseComponent(Registry &registry, Entity bossEntity)
{
    if (!registry.hasComponent<BossPhase>(bossEntity)) {
        registry.addComponent<BossPhase>(bossEntity, {});
    }
}

void BossAttackSystem::updatePhase(Registry &registry, Entity bossEntity)
{
    BossPhase &phase = registry.getComponent<BossPhase>(bossEntity);
    const EnemyStats &stats = registry.getComponent<EnemyStats>(bossEntity);

    if (stats.maxHealth <= 0.0f) {
        return;
    }

    if (stats.health <= (0.5f * stats.maxHealth)) {
        phase.phase = BossPhaseState::Phase2;
    }
}

void BossAttackSystem::spawnRadialProjectileBurst(Registry &registry, const config::GameConfig &config,
                                                  Entity bossEntity)
{
    const Position &bossPosition = registry.getComponent<Position>(bossEntity);
    const view::Sprite &bossSprite = registry.getComponent<view::Sprite>(bossEntity);
    const EnemyStats &bossStats = registry.getComponent<EnemyStats>(bossEntity);

    const config::AttackProfileConfig &attackProfile = config.enemyClasses.boss.attack;
    const config::EnemyClassConfig &bossConfig = config.enemyClasses.boss;
    const config::AnimationFrame projectileFrame = config::AnimationConfigHelper::getProjectileAnimationFrame(
        config, attackProfile.projectile, AnimationState::Idle, AnimationDirection::None, 0);
    const config::SpriteConfig &projectileSpriteConfig = projectileFrame.spriteConfig;

    const geometry::Vec2<float> bossCenter = bossPosition.p + (bossSprite.rect.size / 2.0f);
    const float spawnRadius = std::max(bossSprite.rect.size.x, bossSprite.rect.size.y) / 2.0f;
    const float baseSpeed = bossStats.speedOfAttack > 0.0f ? bossStats.speedOfAttack : defaultProjectileSpeed;
    const float projectileSpeed = std::max(1.0f, baseSpeed * attackProfile.projectile.velocityScale);
    const float projectileRange = bossStats.attackRange > 0.0f ? bossStats.attackRange : defaultProjectileRange;

    for (int i = 0; i < radialProjectileCount; ++i) {
        const float angle = (2.0f * pi * static_cast<float>(i)) / static_cast<float>(radialProjectileCount);
        const geometry::Vec2<float> unitDirection = {std::cos(angle), std::sin(angle)};

        const geometry::Vec2<float> projectileCenter = bossCenter + (unitDirection * spawnRadius);
        const geometry::Vec2<float> projectilePosition =
            projectileCenter - (projectileSpriteConfig.texture.size / 2.0f);

        const Entity projectileEntity = registry.createEntity();

        registry.addComponent<Damage>(
            projectileEntity,
            {.amount = bossStats.attackPower * attackProfile.amount,
             .pushBackForce = attackProfile.pushBackForce,
             .stunChance = attackProfile.stunChance,
             .kind = DamageKind::Projectile,
             .params = ProjectileDamage{
                 .speed = projectileSpeed, .maxRange = projectileRange, .distanceTraveled = 0.0f, .maxTargets = 1}});
        registry.addComponent<Position>(projectileEntity, {projectilePosition});
        registry.addComponent<Velocity>(projectileEntity,
                                        {unitDirection.x * projectileSpeed, unitDirection.y * projectileSpeed});
        registry.addComponent<HitBox>(projectileEntity,
                                      {projectileSpriteConfig.hitBox.offset, projectileSpriteConfig.hitBox.size});
        registry.addComponent<view::Sprite>(projectileEntity,
                                            {.rect = {projectilePosition, projectileSpriteConfig.texture.size},
                                             .imagePath = projectileSpriteConfig.texture.path});
        registry.addComponent<Animation>(projectileEntity, createAttackAnimation(projectileFrame));
        registry.addComponent<DamageTag>(projectileEntity, {});
        registry.addComponent<EnemyAttackTag>(projectileEntity, {bossStats.enemyType});

        if (i == 0 && !bossConfig.sounds.attack.empty()) {
            registry.addComponent<Sound>(projectileEntity, {bossConfig.sounds.attack});
        }
    }
}

void BossAttackSystem::spawnPhaseTwoLightning(Registry &registry, const config::GameConfig &config, Entity bossEntity)
{
    const auto mapEntities = registry.view<MapTag, Position, view::Sprite>();
    const auto playerEntities = registry.view<PlayerTag, Position>();
    if (mapEntities.empty() || playerEntities.empty()) {
        return;
    }

    const Entity mapEntity = mapEntities.front();
    const Entity playerEntity = playerEntities.front();
    const Position &mapPosition = registry.getComponent<Position>(mapEntity);
    const view::Sprite &mapSprite = registry.getComponent<view::Sprite>(mapEntity);
    const Position &playerPosition = registry.getComponent<Position>(playerEntity);

    const config::AttackProfileConfig &attackProfile = config.enemyClasses.boss.attack;
    const config::EnemyClassConfig &bossConfig = config.enemyClasses.boss;
    const EnemyStats &bossStats = registry.getComponent<EnemyStats>(bossEntity);
    const config::AnimationFrame lightningFrame = config::AnimationConfigHelper::getAreaAnimationFrame(
        config, attackProfile.area, AnimationState::Idle, AnimationDirection::None, 0);
    const config::SpriteConfig &lightningSpriteConfig = lightningFrame.spriteConfig;

    const geometry::Vec2<float> spriteSize = lightningSpriteConfig.texture.size;

    const float strikeRadius = std::max(lightningSpriteConfig.texture.size.x, lightningSpriteConfig.texture.size.y);
    const std::array<geometry::Vec2<float>, phaseTwoLightningStrikeCount> strikeOffsets = {
        geometry::Vec2<float>{0.0f, 0.0f},
        geometry::Vec2<float>{strikeRadius * 0.75f, 0.0f},
        geometry::Vec2<float>{-strikeRadius * 0.75f, 0.0f},
    };

    for (int i = 0; i < phaseTwoLightningStrikeCount; ++i) {
        const geometry::Vec2<float> &strikeOffset = strikeOffsets[static_cast<std::size_t>(i)];
        const geometry::Vec2<float> unclampedStrikePosition = {
            playerPosition.p.x + strikeOffset.x - (spriteSize.x / 2.0f),
            playerPosition.p.y + strikeOffset.y - (spriteSize.y / 2.0f),
        };
        const geometry::Vec2<float> strikePosition =
            clampSpritePositionToMap(unclampedStrikePosition, spriteSize, mapPosition.p, mapSprite.rect.size);

        const Entity lightningEntity = registry.createEntity();
        registry.addComponent<Damage>(lightningEntity,
                                      {.amount = bossStats.attackPower * attackProfile.amount,
                                       .pushBackForce = attackProfile.pushBackForce,
                                       .stunChance = attackProfile.stunChance,
                                       .kind = DamageKind::Area,
                                       .params = AreaDamage{.radius = attackProfile.area.radius,
                                                            .activeTimeSec = attackProfile.area.activeTimeSec,
                                                            .elapsedSec = 0.0f,
                                                            .telegraphTimeSec = attackProfile.area.telegraphTimeSec,
                                                            .initialHit = attackProfile.area.initialHit,
                                                            .damageTicks = std::max(1, attackProfile.area.damageTicks),
                                                            .elapsedSecSinceLastTick = 0.0f}});
        registry.addComponent<Position>(lightningEntity, {.p = strikePosition});
        registry.addComponent<HitBox>(lightningEntity, {.offset = lightningSpriteConfig.hitBox.offset,
                                                        .size = lightningSpriteConfig.hitBox.size});
        registry.addComponent<view::Sprite>(
            lightningEntity, {.rect = {strikePosition, spriteSize}, .imagePath = lightningSpriteConfig.texture.path});
        registry.addComponent<Animation>(lightningEntity, createAttackAnimation(lightningFrame));
        registry.addComponent<DamageTag>(lightningEntity, {});
        registry.addComponent<EnemyAttackTag>(lightningEntity, {bossStats.enemyType});

        if (i == 0 && !bossConfig.sounds.special.empty()) {
            registry.addComponent<Sound>(lightningEntity, {bossConfig.sounds.special});
        }
    }
}

void BossAttackSystem::update(Registry &registry, const config::GameConfig &config, float dtSec)
{
    for (const Entity enemy : registry.view<EnemyTag, EnemyStats, Position, view::Sprite>()) {
        const EnemyStats &enemyStats = registry.getComponent<EnemyStats>(enemy);
        if (enemyStats.enemyType != EnemyType::Boss) {
            continue;
        }

        ensurePhaseComponent(registry, enemy);
        updatePhase(registry, enemy);

        if (registry.hasComponent<EnemyAttackCooldown>(enemy)) {
            updateCoolDown(registry, enemy, dtSec);
            continue;
        }

        const float attackSpeed = std::max(0.01f, enemyStats.attackSpeed);
        registry.addComponent<EnemyAttackCooldown>(enemy, {.remainingSec = 1.0f / attackSpeed});

        bool shouldTriggerMainAttack = true;
        if (registry.hasComponent<DistanceToPlayer>(enemy)) {
            const DistanceToPlayer &distance = registry.getComponent<DistanceToPlayer>(enemy);
            const float triggerRange = std::max(minProjectileRangeTrigger, enemyStats.attackRange);
            shouldTriggerMainAttack = distance.hasPlayer && distance.value <= triggerRange;
        }

        if (shouldTriggerMainAttack) {
            const AnimationDirection attackDirection = registry.hasComponent<Animation>(enemy)
                                                           ? registry.getComponent<Animation>(enemy).direction
                                                           : AnimationDirection::Right;
            applyBossAttackAnimation(registry, config, enemy, enemyStats.enemyType, attackDirection);
            spawnRadialProjectileBurst(registry, config, enemy);
        }

        const BossPhase &phase = registry.getComponent<BossPhase>(enemy);
        if (phase.phase == BossPhaseState::Phase2) {
            spawnPhaseTwoLightning(registry, config, enemy);
        }
    }
}

} // namespace game