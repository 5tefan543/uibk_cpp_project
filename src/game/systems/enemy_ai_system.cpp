#include "game/ecs/systems/enemy_ai_system.hpp"
#include "config/animation_config_helper.hpp"
#include "config/game_config.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/enemy_attack_tag.hpp"
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

void setAnimationState(Animation &animation, AnimationState state, AnimationDirection direction)
{
    if (animation.state == state && animation.direction == direction) {
        return;
    }

    animation.state = state;
    animation.direction = direction;
    animation.currentFrame = 0;
    animation.frameTimer = 0.0f;
}

void startTimedAnimation(Animation &animation, AnimationState state, AnimationDirection direction, float durationSec)
{
    animation.state = state;
    animation.direction = direction;
    animation.currentFrame = 0;
    animation.frameTimer = 0.0f;
    animation.stateTimeRemaining = durationSec;
}

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

void EnemyAI::updateCoolDowns(Registry &registry, Entity enemyEntity, float dt)
{
    EnemyStats &enemyStats = registry.getComponent<EnemyStats>(enemyEntity);
    if (attackCoolDowns_.contains(enemyEntity)) {
        auto it = attackCoolDowns_.find(enemyEntity);
        if (it != attackCoolDowns_.end()) {
            it->second -= dt;
        }
        if (it->second <= 0.0f) {
            attackCoolDowns_.erase(enemyEntity);
        }
    }
}

void EnemyAI::update(Registry &registry, const config::GameConfig &config, LocationTable &locationTable, float dt)
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
        updateEnemyAnimationState(registry, enemy, dt);
        updateAttack(registry, config, enemy, playerPos);
        applyAnimationMoveSpeedModifier(registry, config, enemy);
        updateCoolDowns(registry, enemy, dt);
    }
}

void EnemyAI::updateAttack(Registry &registry, const config::GameConfig &config, Entity enemy,
                           const Position &playerPos)
{
    if (attackCoolDowns_.contains(enemy)) {
        return;
    }
    attackCoolDowns_[enemy] = 1 / config.enemyClasses.blob.stats.attackSpeed;

    blobAreaAttack(registry, config, enemy, playerPos);
}

void EnemyAI::updateEnemyVelocityTowardsPlayer(Registry &registry, LocationTable &locationTable,
                                               const Position &playerPos, Entity enemy)
{
    const Vec2 playerPosVec{playerPos.x, playerPos.y};

    auto &[vx, vy] = registry.getComponent<Velocity>(enemy);
    auto &[px, py] = registry.getComponent<Position>(enemy);

    EnemyStats &enemyStats = registry.getComponent<EnemyStats>(enemy);
    if (enemyStats.moveSpeed == 0) {
        vx = 0;
        vy = 0;
        return;
    }
    Vec2 enemyPosVec{px, py};

    // Set movement direction exactly towards player
    Vec2 v = playerPosVec - enemyPosVec;

    // Prevent shooting over target (player) position
    if (v.length() < minDistanceEnemyPlayer) {
        vx = 0; // TODO: into Vec2
        vy = 0; // TODO: into Vec2
        return;
    }

    // Calc. repelling force between enemies
    auto enemiesInRange = locationTable.getEntitiesInRange(enemyPosVec, enemyRepelRadius, registry);
    Vec2<float> repelOffset = {0, 0};
    for (auto [e, position] : enemiesInRange) {
        if (e == enemy) {
            continue;
        }
        const auto p = Vec2{position.x, position.y}; // TODO: into Vec2
        const auto pToOther = (enemyPosVec - p);
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
        vx = 0; // TODO: into Vec2
        vy = 0; // TODO: into Vec2
        return;
    }
    v.setLength(l);

    vx = v.x; // TODO: into Vec2
    vy = v.y; // TODO: into Vec2
}

void EnemyAI::updateEnemyAnimationState(Registry &registry, Entity enemy, float dt)
{
    Animation &animation = registry.getComponent<Animation>(enemy);
    const Velocity &velocity = registry.getComponent<Velocity>(enemy);

    if (animation.stateTimeRemaining > 0.0f) {
        animation.stateTimeRemaining = std::max(0.0f, animation.stateTimeRemaining - dt);
    }

    if (animation.stateTimeRemaining > 0.0f) {
        return;
    }

    const bool isMoving = std::abs(velocity.x) > 0.1f || std::abs(velocity.y) > 0.1f;
    const AnimationState nextState = isMoving ? AnimationState::Walk : AnimationState::Idle;

    AnimationDirection direction = animation.direction;

    const auto absVelocity = Vec2{velocity.x, velocity.y}.abs();
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
    Velocity &enemyVelocity = registry.getComponent<Velocity>(enemyEntity);

    const config::AnimationFrame currentFrame = config::AnimationConfigHelper::getEnemyAnimationFrame(
        config, enemyStats.enemyType, enemyAnimation.state, enemyAnimation.direction, enemyAnimation.currentFrame);

    enemyVelocity.x *= currentFrame.moveSpeedMultiplier;
    enemyVelocity.y *= currentFrame.moveSpeedMultiplier;
}

void EnemyAI::blobAreaAttack(Registry &registry, const config::GameConfig &config, Entity blobEntity,
                             const Position &playerPosition)
{
    const Vec2 playerPosVec{playerPosition.x, playerPosition.y};
    const Position blobPosition = registry.getComponent<Position>(blobEntity);
    Vec2 enemyPosVec{blobPosition.x, blobPosition.y};

    Vec2 v = playerPosVec - enemyPosVec;
    const config::AttackProfileConfig &attackProfile = config.enemyClasses.blob.attack;
    const EnemyStats &enemyStats = registry.getComponent<EnemyStats>(blobEntity);

    if (v.length() >= (enemyStats.attackRange * attackProfile.area.radius)) {
        return;
    }

    const AnimationDirection attackDirection =
        playerPosition.x >= blobPosition.x ? AnimationDirection::Right : AnimationDirection::Left;
    applyAnimation(registry, config, blobEntity, AnimationState::Attack, enemyStats.enemyType, attackDirection);

    pendingAreaSpawns_.insert(blobEntity);
}

void EnemyAI::spawnPendingAreaAttack(Registry &registry, const config::GameConfig &config, Entity blobEntity)
{
    if (!pendingAreaSpawns_.contains(blobEntity)) {
        return;
    }

    const Animation &animation = registry.getComponent<Animation>(blobEntity);
    if (animation.stateTimeRemaining > 0.0f) {
        return;
    }

    pendingAreaSpawns_.erase(blobEntity);

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
    const Position damagePosition{blobPosition.x, blobPosition.y};
    const view::Sprite sprite{damagePosition.x, damagePosition.y, areaSpriteConfig.texture.path,
                              areaSpriteConfig.texture.size.x * enemyStats.attackRange,
                              areaSpriteConfig.texture.size.y * enemyStats.attackRange};
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