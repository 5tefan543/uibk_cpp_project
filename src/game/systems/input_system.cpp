#include "game/ecs/systems/input_system.hpp"
#include "config/animation_config_helper.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/player_attack_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/location_table.hpp"
#include "view/sprite.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

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
                     const AnimationState attackState, const CharacterType characterType,
                     const AnimationDirection direction)
{
    Animation &playerAnimation = registry.getComponent<Animation>(entity);

    const config::AnimationFrame firstPlayerFrame =
        config::AnimationConfigHelper::getPlayerAnimationFrame(config, characterType, attackState, direction, 0);

    const float animationDuration = static_cast<float>(firstPlayerFrame.totalFrames) * firstPlayerFrame.frameDuration;
    startTimedAnimation(playerAnimation, attackState, direction, animationDuration);

    return animationDuration;
}

} // namespace

void InputSystem::update(Registry &registry, const config::GameConfig &config, const controller::InputState &input,
                         float dt)
{
    updateCooldown(dt);

    auto players = registry.view<PlayerTag, Velocity, PlayerStats, view::Sprite, Position, Animation>();

    if (players.empty()) {
        return;
    }

    if (players.size() > 1) {
        logger::log(logger::LogLevel::WARNING,
                    "Multiple player entities found in the registry. Only the first one will be processed.");
    }

    Entity playerEntity = players.front();

    updatePlayerVelocity(registry, playerEntity, input);
    updatePlayerAnimationState(registry, playerEntity, dt);
    handleAttack(registry, config, playerEntity, input);
    applyAnimationMoveSpeedModifier(registry, config, playerEntity);
}

void InputSystem::updateCooldown(float dt)
{
    // possible since we have only one player
    timeSinceLastAttack_ += dt;
    timeSinceLastSpecialMove_ += dt;
    timeSinceLastDash_ += dt;
}

void InputSystem::updatePlayerVelocity(Registry &registry, const Entity entity, const controller::InputState &input)
{
    Velocity &velocity = registry.getComponent<Velocity>(entity);
    const PlayerStats &playerStats = registry.getComponent<PlayerStats>(entity);

    Vec2<float> v = {0, 0};

    if (input.leftHeld) {
        v.x -= playerStats.moveSpeed;
    }
    if (input.rightHeld) {
        v.x += playerStats.moveSpeed;
    }
    if (input.upHeld) {
        v.y -= playerStats.moveSpeed;
    }
    if (input.downHeld) {
        v.y += playerStats.moveSpeed;
    }
    v.setLength(playerStats.moveSpeed);
    velocity.x = v.x;
    velocity.y = v.y; // TODO: into Vec2
}

void InputSystem::updatePlayerAnimationState(Registry &registry, Entity enemy, float dt)
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

void InputSystem::handleAttack(Registry &registry, const config::GameConfig &config, Entity playerEntity,
                               const controller::InputState &input)
{
    PlayerStats &stats = registry.getComponent<PlayerStats>(playerEntity);

    if (timeSinceLastAttack_ <= 1.0f / stats.attackSpeed) {
        return;
    }

    if (registry.hasComponent<Animation>(playerEntity)) {
        const Animation &animation = registry.getComponent<Animation>(playerEntity);
        if (animation.stateTimeRemaining > 0.0f && animation.state == AnimationState::Attack) {
            // if the player is in the middle of an attack animation, we don't want to interrupt it
            return;
        }
    }

    const config::AttackProfileConfig &attackProfile = config.playerClasses.getByType(stats.characterType).attack;

    if (input.mouseLeftPressed) {
        if (attackProfile.kind == DamageKind::MeleeArc) {
            attackMelee(registry, config, playerEntity, input, attackProfile);
            timeSinceLastAttack_ = 0.0f;
        } else if (attackProfile.kind == DamageKind::Projectile) {
            attackRanged(registry, config, playerEntity, input, attackProfile);
            timeSinceLastAttack_ = 0.0f;
        }
        return; // only handle one attack per update
    }

    if (input.mouseRightPressed) {
        if (attackProfile.kind == DamageKind::MeleeArc) {
            // attackMeleeSpecialMove(registry, config, playerEntity, input, attackProfile);
            timeSinceLastAttack_ = 0.0f;
        } else if (attackProfile.kind == DamageKind::Projectile) {
            // attackRangedSpecialMove(registry, config, playerEntity, input, attackProfile);
            timeSinceLastAttack_ = 0.0f;
        }
        return; // only handle one attack per update
    }
}

void InputSystem::attackMelee(Registry &registry, const config::GameConfig &config, Entity playerEntity,
                              const controller::InputState &input, const config::AttackProfileConfig &attackProfile)
{
    const Position playerPosition = registry.getComponent<Position>(playerEntity);
    const PlayerStats &playerStats = registry.getComponent<PlayerStats>(playerEntity);
    const view::Sprite playerSprite = registry.getComponent<view::Sprite>(playerEntity);

    const AnimationDirection attackDirection =
        input.mouseGridX >= playerPosition.x ? AnimationDirection::Right : AnimationDirection::Left;

    const float animationDuration = applyAnimation(registry, config, playerEntity, AnimationState::Attack,
                                                   playerStats.characterType, attackDirection);

    // create melee attack entity
    const Damage damageComponent{.amount = attackProfile.amount,
                                 .pushBackForce = attackProfile.pushBackForce,
                                 .stunChance = attackProfile.stunChance,
                                 .kind = attackProfile.kind,
                                 .params = MeleeArcDamage{
                                     .reach = attackProfile.meleeArc.reach,
                                     .activeTimeSec = animationDuration + attackProfile.meleeArc.activeTimePaddingSec,
                                     .elapsedSec = 0.0f,
                                 }};

    const Position damagePosition{playerPosition.x, playerPosition.y};

    const float hitBoxOffsetX =
        attackDirection == AnimationDirection::Right ? playerSprite.width / 2 : -playerSprite.width / 2;
    const float hitBoxOffsetY = -(attackProfile.meleeArc.reach) * playerStats.attackRange;

    const HitBox meleeHitBox{
        .offset = {hitBoxOffsetX, hitBoxOffsetY},
        .size = {attackProfile.meleeArc.hitBoxWidth + 2 * attackProfile.meleeArc.reach * playerStats.attackRange,
                 attackProfile.meleeArc.hitBoxHeight + 2 * attackProfile.meleeArc.reach * playerStats.attackRange}};

    // add melee attack entity with all components
    // component references may be invalid: retrieve again from registry if used after this point
    const Entity meleeAttackEntity = registry.createEntity();
    registry.addComponent<Damage>(meleeAttackEntity, damageComponent);
    registry.addComponent<DamageTag>(meleeAttackEntity, {});
    registry.addComponent<Position>(meleeAttackEntity, damagePosition);
    registry.addComponent<HitBox>(meleeAttackEntity, meleeHitBox);
    registry.addComponent<PlayerAttackTag>(meleeAttackEntity, {}); // Mark as player's attack for collision detection
}

void InputSystem::attackRanged(Registry &registry, const config::GameConfig &config, Entity playerEntity,
                               const controller::InputState &input, const config::AttackProfileConfig &attackProfile)
{
    const Position playerPosition = registry.getComponent<Position>(playerEntity);
    const PlayerStats &playerStats = registry.getComponent<PlayerStats>(playerEntity);
    const view::Sprite &playerSprite = registry.getComponent<view::Sprite>(playerEntity);

    const AnimationDirection attackDirection =
        input.mouseGridX >= playerPosition.x ? AnimationDirection::Right : AnimationDirection::Left;

    applyAnimation(registry, config, playerEntity, AnimationState::Attack, playerStats.characterType, attackDirection);

    // add projectile

    const config::AnimationFrame projectileFrame = config::AnimationConfigHelper::getProjectileAnimationFrame(
        config, attackProfile.projectile, AnimationState::Idle, AnimationDirection::None, 0);
    const config::SpriteConfig &projectileSpriteConfig = projectileFrame.spriteConfig;

    const float projectileOffsetX =
        attackDirection == AnimationDirection::Right ? playerSprite.width : -projectileSpriteConfig.texture.size.x;
    const Position projectileLaunchPosition{.x = playerPosition.x + projectileOffsetX,
                                            .y = playerPosition.y + (playerSprite.height / 2)
                                                 - (projectileSpriteConfig.texture.size.y / 2)};

    const float projectileLaunchAngle =
        std::atan2(input.mouseGridY - projectileLaunchPosition.y, input.mouseGridX - projectileLaunchPosition.x);
    const Velocity projectileLaunchVelocity{
        .x = playerStats.speedOfAttack * attackProfile.projectile.velocityScale * std::cos(projectileLaunchAngle),
        .y = playerStats.speedOfAttack * attackProfile.projectile.velocityScale * std::sin(projectileLaunchAngle)};

    const Damage projectileDamage{.amount = attackProfile.amount,
                                  .pushBackForce = attackProfile.pushBackForce,
                                  .stunChance = attackProfile.stunChance,
                                  .kind = attackProfile.kind,
                                  .params = ProjectileDamage{
                                      .speed = playerStats.speedOfAttack,
                                      .maxRange = playerStats.attackRange,
                                      .distanceTraveled = 0.0f,
                                      .maxTargets = 1,
                                  }};

    const view::Sprite projectileSprite{.x = projectileLaunchPosition.x,
                                        .y = projectileLaunchPosition.y,
                                        .imagePath = projectileSpriteConfig.texture.path,
                                        .width = projectileSpriteConfig.texture.size.x,
                                        .height = projectileSpriteConfig.texture.size.y};

    const HitBox projectileHitBox{.offset = projectileSpriteConfig.hitBox.offset,
                                  .size = projectileSpriteConfig.hitBox.size};

    // add projectile entity with all components
    // component references may be invalid: retrieve again from registry if used after this point
    const Entity projectileEntity = registry.createEntity();
    registry.addComponent<Damage>(projectileEntity, projectileDamage);
    registry.addComponent<view::Sprite>(projectileEntity, projectileSprite);
    registry.addComponent<Position>(projectileEntity, projectileLaunchPosition);
    registry.addComponent<Velocity>(projectileEntity, projectileLaunchVelocity);
    registry.addComponent<HitBox>(projectileEntity, projectileHitBox);
    registry.addComponent<PlayerAttackTag>(projectileEntity, {}); // Mark as player's attack for collision detection
    registry.addComponent<DamageTag>(projectileEntity, {});
}

void InputSystem::applyAnimationMoveSpeedModifier(Registry &registry, const config::GameConfig &config,
                                                  Entity playerEntity)
{
    const Animation &playerAnimation = registry.getComponent<Animation>(playerEntity);
    const PlayerStats &playerStats = registry.getComponent<PlayerStats>(playerEntity);
    Velocity &playerVelocity = registry.getComponent<Velocity>(playerEntity);

    const config::AnimationFrame currentFrame =
        config::AnimationConfigHelper::getPlayerAnimationFrame(config, playerStats.characterType, playerAnimation.state,
                                                               playerAnimation.direction, playerAnimation.currentFrame);

    playerVelocity.x *= currentFrame.moveSpeedMultiplier;
    playerVelocity.y *= currentFrame.moveSpeedMultiplier;
}

} // namespace game