#include "game/ecs/systems/blob_attack_system.hpp"
#include "config/animation_config_helper.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/distance_to_player.hpp"
#include "game/ecs/components/enemy_attack_cooldown.hpp"
#include "game/ecs/components/enemy_attack_tag.hpp"
#include "game/ecs/components/enemy_pending_area_spawn_tag.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "view/sprite.hpp"

namespace game {

namespace {

float applyAnimation(Registry &registry, const config::GameConfig &config, const Entity entity,
                     const AnimationState attackState, const EnemyType enemyType, const AnimationDirection direction)
{
    Animation &enemyAnimation = registry.getComponent<Animation>(entity);

    const config::AnimationFrame firstEnemyFrame =
        config::AnimationConfigHelper::getEnemyAnimationFrame(config, enemyType, attackState, direction, 0);

    const float animationDuration = static_cast<float>(firstEnemyFrame.totalFrames) * firstEnemyFrame.frameDuration;
    startTimedAnimation(enemyAnimation, attackState, direction, animationDuration);

    return animationDuration;
}

} // namespace

void BlobAttackSystem::updateCoolDowns(Registry &registry, Entity enemyEntity, float dtSec)
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

void BlobAttackSystem::triggerBlobAreaAttack(Registry &registry, const config::GameConfig &config, Entity blobEntity,
                                             const Position &playerPosition)
{
    if (!registry.hasComponent<DistanceToPlayer>(blobEntity)) {
        return;
    }

    const DistanceToPlayer &distance = registry.getComponent<DistanceToPlayer>(blobEntity);
    if (!distance.hasPlayer) {
        return;
    }

    const config::AttackProfileConfig &attackProfile = config.enemyClasses.blob.attack;
    const EnemyStats &enemyStats = registry.getComponent<EnemyStats>(blobEntity);

    if (distance.value >= (enemyStats.attackRange * attackProfile.area.radius)) {
        return;
    }

    const Position blobPosition = registry.getComponent<Position>(blobEntity);
    const AnimationDirection attackDirection =
        playerPosition.p.x >= blobPosition.p.x ? AnimationDirection::Right : AnimationDirection::Left;
    applyAnimation(registry, config, blobEntity, AnimationState::Attack, enemyStats.enemyType, attackDirection);

    if (!registry.hasComponent<EnemyPendingAreaSpawnTag>(blobEntity)) {
        registry.addComponent<EnemyPendingAreaSpawnTag>(blobEntity, {});
    }
}

void BlobAttackSystem::spawnPendingAreaAttack(Registry &registry, const config::GameConfig &config, Entity blobEntity)
{
    if (!registry.hasComponent<EnemyPendingAreaSpawnTag>(blobEntity)) {
        return;
    }

    const Animation &animation = registry.getComponent<Animation>(blobEntity);
    if (animation.stateTimeRemaining > 0.0f) {
        return;
    }

    registry.removeComponent<EnemyPendingAreaSpawnTag>(blobEntity);

    const config::AttackProfileConfig &attackProfile = config.enemyClasses.blob.attack;
    const EnemyStats &enemyStats = registry.getComponent<EnemyStats>(blobEntity);
    const Position blobPosition = registry.getComponent<Position>(blobEntity);

    const Damage damageComponent{.amount = attackProfile.amount,
                                 .pushBackForce = attackProfile.pushBackForce,
                                 .stunChance = attackProfile.stunChance,
                                 .kind = DamageKind::Area,
                                 .params = AreaDamage{.radius = attackProfile.area.radius,
                                                      .activeTimeSec = attackProfile.area.activeTimeSec,
                                                      .elapsedSec = 0.0f,
                                                      .initialHit = attackProfile.area.initialHit,
                                                      .damageTicks = attackProfile.area.damageTicks,
                                                      .elapsedSecSinceLastTick = 0.0f}};
    const config::AnimationFrame areaFrame = config::AnimationConfigHelper::getAreaAnimationFrame(
        config, attackProfile.area, AnimationState::Idle, AnimationDirection::None, 0);
    const config::SpriteConfig &areaSpriteConfig = areaFrame.spriteConfig;
    const Position damagePosition{blobPosition.p.x, blobPosition.p.y};
    const view::Sprite sprite{.rect = {damagePosition.p,
                                       {areaSpriteConfig.texture.size.x * enemyStats.attackRange,
                                        areaSpriteConfig.texture.size.y * enemyStats.attackRange}},
                              .imagePath = areaSpriteConfig.texture.path};
    const Animation areaAnimation{};

    const HitBox areaHitbox{.offset = {areaSpriteConfig.hitBox.offset.x, areaSpriteConfig.hitBox.offset.y},
                            .size = {areaSpriteConfig.hitBox.size.x * enemyStats.attackRange,
                                     areaSpriteConfig.hitBox.size.y * enemyStats.attackRange}};

    const Entity areaAttackEntity = registry.createEntity();
    registry.addComponent<Damage>(areaAttackEntity, damageComponent);
    registry.addComponent<DamageTag>(areaAttackEntity, {});
    registry.addComponent<Position>(areaAttackEntity, damagePosition);
    registry.addComponent<HitBox>(areaAttackEntity, areaHitbox);
    registry.addComponent<view::Sprite>(areaAttackEntity, sprite);
    registry.addComponent<Animation>(areaAttackEntity, areaAnimation);
    registry.addComponent<EnemyAttackTag>(areaAttackEntity, {blobEntity});
}

void BlobAttackSystem::update(Registry &registry, const config::GameConfig &config, float dtSec)
{
    const auto players = registry.view<PlayerTag, Position>();
    if (players.empty()) {
        return;
    }

    const Position &playerPos = registry.getComponent<Position>(players.front());
    const float attackSpeed = std::max(0.01f, config.enemyClasses.blob.stats.attackSpeed);

    for (const Entity enemy : registry.view<EnemyTag, EnemyStats, Position, Animation>()) {
        const EnemyStats &enemyStats = registry.getComponent<EnemyStats>(enemy);
        if (enemyStats.enemyType != EnemyType::Blob) {
            continue;
        }

        spawnPendingAreaAttack(registry, config, enemy);

        if (!registry.hasComponent<EnemyAttackCooldown>(enemy)) {
            registry.addComponent<EnemyAttackCooldown>(enemy, {.remainingSec = 1.0f / attackSpeed});
            triggerBlobAreaAttack(registry, config, enemy, playerPos);
        }

        updateCoolDowns(registry, enemy, dtSec);
    }
}

} // namespace game