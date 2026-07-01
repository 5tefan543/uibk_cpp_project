#include "game/ecs/systems/enemy_ai_system.hpp"
#include "config/animation_config_helper.hpp"
#include "config/game_config.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/enemy_attack_cooldown.hpp"
#include "game/ecs/components/enemy_attack_tag.hpp"
#include "game/ecs/components/enemy_pending_area_spawn_tag.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/sound.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/location_table.hpp"
#include "view/sprite.hpp"
#include <cmath>
#include <format>

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

const float minDistanceEnemyPlayer = 5;
const float enemyRepelRadius = 50;
const float enemyRepelProximityRampParam = 1.5;
const float enemyRelSpeedCutoffPercentage = 0.02;

void EnemyAI::updateCoolDowns(Registry &registry, Entity enemyEntity, float dtSec)
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

void EnemyAI::update(Registry &registry, const config::GameConfig &config, LocationTable &locationTable, float dtSec)
{
    const auto players = registry.view<PlayerTag, Position>();
    if (players.empty()) {
        return;
    }
    const auto player = players.front();
    const Position &playerPos = registry.getComponent<Position>(player);

    for (auto enemy : registry.view<EnemyTag, Velocity, EnemyStats, Position, Animation>()) {
        spawnPendingAreaAttack(registry, config, enemy);
        updateEnemyVelocityTowardsPlayer(registry, locationTable, playerPos, enemy);
        updateEnemyAnimationState(registry, enemy, dtSec);
        updateAttack(registry, config, enemy, playerPos);
        applyAnimationMoveSpeedModifier(registry, config, enemy);
        updateCoolDowns(registry, enemy, dtSec);
    }
}

void EnemyAI::updateAttack(Registry &registry, const config::GameConfig &config, Entity enemy,
                           const Position &playerPos)
{
    if (registry.hasComponent<EnemyAttackCooldown>(enemy)) {
        return;
    }
    registry.addComponent<EnemyAttackCooldown>(enemy,
                                               {
                                                   .remainingSec = 1 / config.enemyClasses.blob.stats.attackSpeed,
                                               });

    blobAreaAttack(registry, config, enemy, playerPos);
}

void EnemyAI::updateEnemyVelocityTowardsPlayer(Registry &registry, LocationTable &locationTable,
                                               const Position &playerPosition, Entity enemy)
{
    using geometry::Vec2;

    const float minDistanceEnemyPlayer = 5;
    const float enemyRepelRadius = 50;
    const float enemyRepelProximityRampParam = 1.5;
    const float enemyRelSpeedCutoffPercentage = 0.02;

    const Vec2<float> &playerPos = playerPosition.p;

    Vec2<float> &v = registry.getComponent<Velocity>(enemy).v;
    const Vec2<float> &enemyPos = registry.getComponent<Position>(enemy).p;

    EnemyStats &enemyStats = registry.getComponent<EnemyStats>(enemy);
    if (enemyStats.moveSpeed == 0) {
        v = {0, 0};
        return;
    }

    // Set movement direction exactly towards player
    v = playerPos - enemyPos;

    // TODO: add player attack: -> stwa: maybe not in this method and similar to input system ?
    // if (v.length() < 30) {attack_player();}

    // Prevent shooting over target (player) position
    if (v.length() < minDistanceEnemyPlayer) {
        v = {0, 0};
        return;
    }

    // Calc. repelling force between enemies
    auto enemiesInRange = locationTable.getEntitiesInRange(enemyPos, enemyRepelRadius, registry);
    Vec2<float> repelOffset = {0, 0};
    for (const auto &[otherEnemy, otherPos] : enemiesInRange) {
        if (otherEnemy == enemy) {
            continue;
        }
        const auto pToOther = (enemyPos - otherPos.p);
        const auto b = (std::pow(pToOther.length(), enemyRepelProximityRampParam));
        if (b != 0) {
            repelOffset += pToOther / b; // increase repelling with proximity
        }
    }

    // Divert straight forwards movement to player with repelling offset
    v.normalize();
    v += repelOffset;
    v *= enemyStats.moveSpeed;

    // Enforce enemy speed limit if repelling force would boost it over max
    auto l = std::min(v.length(), enemyStats.moveSpeed);

    // Prevents jittery movement in enemy heaps by
    // - decreasing speed inverse propotional to max speed
    // - and stopping movement below a certaing percentage
    l *= std::pow(l / enemyStats.moveSpeed, 2);
    if (l / enemyStats.moveSpeed < enemyRelSpeedCutoffPercentage) {
        v = {0, 0};
        return;
    }
    v.setLength(l);
}

void EnemyAI::updateEnemyAnimationState(Registry &registry, Entity enemy, float dtSec)
{
    using geometry::Vec2;

    Animation &animation = registry.getComponent<Animation>(enemy);
    const auto &velocity = registry.getComponent<Velocity>(enemy).v;

    if (animation.stateTimeRemaining > 0.0f) {
        animation.stateTimeRemaining = std::max(0.0f, animation.stateTimeRemaining - dtSec);
    }

    if (animation.stateTimeRemaining > 0.0f) {
        return;
    }

    const auto absVelocity = velocity.abs();
    const bool isMoving = (absVelocity > Vec2{0.1f, 0.1f}).some();
    const AnimationState nextState = isMoving ? AnimationState::Walk : AnimationState::Idle;

    AnimationDirection direction = animation.direction;

    bool isHorizMove = absVelocity.x >= absVelocity.y;

    if (isHorizMove && absVelocity.x > 0.1f) {
        direction = velocity.x > 0.0f ? AnimationDirection::Right : AnimationDirection::Left;
    }

    setAnimationState(animation, nextState, direction);
}

void EnemyAI::applyAnimationMoveSpeedModifier(Registry &registry, const config::GameConfig &config, Entity enemyEntity)
{
    const Animation &enemyAnimation = registry.getComponent<Animation>(enemyEntity);
    const EnemyStats &enemyStats = registry.getComponent<EnemyStats>(enemyEntity);
    auto &enemyVelocity = registry.getComponent<Velocity>(enemyEntity).v;

    const config::AnimationFrame currentFrame = config::AnimationConfigHelper::getEnemyAnimationFrame(
        config, enemyStats.enemyType, enemyAnimation.state, enemyAnimation.direction, enemyAnimation.currentFrame);

    enemyVelocity *= currentFrame.moveSpeedMultiplier;
}

void EnemyAI::blobAreaAttack(Registry &registry, const config::GameConfig &config, Entity blobEntity,
                             const Position &playerPosition)
{
    const geometry::Vec2 playerPosVec{playerPosition.p.x, playerPosition.p.y};
    const Position blobPosition = registry.getComponent<Position>(blobEntity);
    geometry::Vec2 enemyPosVec{blobPosition.p.x, blobPosition.p.y};

    geometry::Vec2 v = playerPosVec - enemyPosVec;
    const config::AttackProfileConfig &attackProfile = config.enemyClasses.blob.attack;
    const EnemyStats &enemyStats = registry.getComponent<EnemyStats>(blobEntity);

    if (v.length() >= (enemyStats.attackRange * attackProfile.area.radius)) {
        return;
    }

    const AnimationDirection attackDirection =
        playerPosition.p.x >= blobPosition.p.x ? AnimationDirection::Right : AnimationDirection::Left;
    applyAnimation(registry, config, blobEntity, AnimationState::Attack, enemyStats.enemyType, attackDirection);

    if (!registry.hasComponent<EnemyPendingAreaSpawnTag>(blobEntity)) {
        registry.addComponent<EnemyPendingAreaSpawnTag>(blobEntity, {});
    }
}

void EnemyAI::spawnPendingAreaAttack(Registry &registry, const config::GameConfig &config, Entity blobEntity)
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
                                 .kind = attackProfile.kind,
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

    // component references may be invalid: retrieve again from registry if used after this point
    const Entity areaAttackEntity = registry.createEntity();
    registry.addComponent<Damage>(areaAttackEntity, damageComponent);
    registry.addComponent<DamageTag>(areaAttackEntity, {});
    registry.addComponent<Position>(areaAttackEntity, damagePosition);
    registry.addComponent<HitBox>(areaAttackEntity, areaHitbox);
    registry.addComponent<view::Sprite>(areaAttackEntity, sprite);
    registry.addComponent<Animation>(areaAttackEntity, areaAnimation);
    registry.addComponent<EnemyAttackTag>(areaAttackEntity,
                                          {blobEntity}); // Mark as enemy's attack for collision detection
}

} // namespace game