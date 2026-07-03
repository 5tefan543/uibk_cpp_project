#include "game/ecs/systems/boss_attack_system.hpp"
#include "config/animation_config_helper.hpp"
#include "game/ecs/components/boss_phase.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/distance_to_player.hpp"
#include "game/ecs/components/enemy_attack_cooldown.hpp"
#include "game/ecs/components/enemy_attack_tag.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "view/sprite.hpp"
#include <algorithm>
#include <cmath>

namespace game {

namespace {

constexpr float pi = 3.14159265358979323846f;
constexpr int radialProjectileCount = 12;
constexpr int phaseTwoLightningStrikeCount = 3;
constexpr float defaultProjectileSpeed = 250.0f;
constexpr float defaultProjectileRange = 800.0f;
constexpr float minProjectileRangeTrigger = 256.0f;

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
            {.amount = attackProfile.amount,
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
        registry.addComponent<DamageTag>(projectileEntity, {});
        registry.addComponent<EnemyAttackTag>(projectileEntity, {bossEntity});
    }
}

void BossAttackSystem::spawnPhaseTwoLightning(Registry &registry, const config::GameConfig &config, Entity bossEntity)
{
    const auto mapEntities = registry.view<MapTag, Position, view::Sprite>();
    if (mapEntities.empty()) {
        return;
    }

    const Entity mapEntity = mapEntities.front();
    const Position &mapPosition = registry.getComponent<Position>(mapEntity);
    const view::Sprite &mapSprite = registry.getComponent<view::Sprite>(mapEntity);

    const config::AttackProfileConfig &attackProfile = config.enemyClasses.boss.attack;
    const EnemyStats &bossStats = registry.getComponent<EnemyStats>(bossEntity);
    const config::AnimationFrame lightningFrame = config::AnimationConfigHelper::getAreaAnimationFrame(
        config, attackProfile.area, AnimationState::Idle, AnimationDirection::None, 0);
    const config::SpriteConfig &lightningSpriteConfig = lightningFrame.spriteConfig;

    const float scale = std::max(1.0f, bossStats.attackRange);
    const geometry::Vec2<float> spriteSize = lightningSpriteConfig.texture.size * scale;

    std::uniform_real_distribution<float> distX(mapPosition.p.x,
                                                mapPosition.p.x + mapSprite.rect.size.x - spriteSize.x);
    std::uniform_real_distribution<float> distY(mapPosition.p.y,
                                                mapPosition.p.y + mapSprite.rect.size.y - spriteSize.y);

    for (int i = 0; i < phaseTwoLightningStrikeCount; ++i) {
        const geometry::Vec2<float> strikePosition{distX(randomEngine_), distY(randomEngine_)};

        const Entity lightningEntity = registry.createEntity();
        registry.addComponent<Damage>(lightningEntity,
                                      {.amount = attackProfile.amount,
                                       .pushBackForce = attackProfile.pushBackForce,
                                       .stunChance = attackProfile.stunChance,
                                       .kind = DamageKind::Area,
                                       .params = AreaDamage{.radius = attackProfile.area.radius,
                                                            .activeTimeSec = attackProfile.area.activeTimeSec,
                                                            .elapsedSec = 0.0f,
                                                            .initialHit = attackProfile.area.initialHit,
                                                            .damageTicks = std::max(1, attackProfile.area.damageTicks),
                                                            .elapsedSecSinceLastTick = 0.0f}});
        registry.addComponent<Position>(lightningEntity, {strikePosition});
        registry.addComponent<HitBox>(lightningEntity, {.offset = lightningSpriteConfig.hitBox.offset,
                                                        .size = lightningSpriteConfig.hitBox.size * scale});
        registry.addComponent<view::Sprite>(
            lightningEntity, {.rect = {strikePosition, spriteSize}, .imagePath = lightningSpriteConfig.texture.path});
        registry.addComponent<DamageTag>(lightningEntity, {});
        registry.addComponent<EnemyAttackTag>(lightningEntity, {bossEntity});
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
            spawnRadialProjectileBurst(registry, config, enemy);
        }

        const BossPhase &phase = registry.getComponent<BossPhase>(enemy);
        if (phase.phase == BossPhaseState::Phase2) {
            spawnPhaseTwoLightning(registry, config, enemy);
        }
    }
}

} // namespace game