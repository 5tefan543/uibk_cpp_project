#include "game/ecs/systems/input_system.hpp"
#include "config/animation_config_helper.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/player_attack_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/sound.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/location_table.hpp"
#include "view/sprite.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace game {

namespace {

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
                         float dtSec)
{
    updateCooldown(dtSec);

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
    updatePlayerAnimationState(registry, playerEntity, dtSec);
    handleAttack(registry, config, playerEntity, input);
    applyAnimationMoveSpeedModifier(registry, config, playerEntity);
}

void InputSystem::updateCooldown(float dtSec)
{
    // possible since we have only one player
    timeSinceLastAttack_ += dtSec;
    timeSinceLastSpecialMove_ += dtSec;
    timeSinceLastDash_ += dtSec;
}

void InputSystem::updatePlayerVelocity(Registry &registry, const Entity entity, const controller::InputState &input)
{
    auto &v = registry.getComponent<Velocity>(entity).v;
    const PlayerStats &playerStats = registry.getComponent<PlayerStats>(entity);

    v = {0, 0};

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
}

void InputSystem::updatePlayerAnimationState(Registry &registry, Entity enemy, float dtSec)
{
    Animation &animation = registry.getComponent<Animation>(enemy);
    const auto &velocity = registry.getComponent<Velocity>(enemy).v;

    if (animation.stateTimeRemaining > 0.0f) {
        animation.stateTimeRemaining = std::max(0.0f, animation.stateTimeRemaining - dtSec);
    }

    if (animation.stateTimeRemaining > 0.0f) {
        return;
    }

    const auto absVelocity = velocity.abs();
    const bool isMoving = (absVelocity > geometry::Vec2{0.1f, 0.1f}).some();
    const AnimationState nextState = isMoving ? AnimationState::Walk : AnimationState::Idle;

    AnimationDirection direction = animation.direction;

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
    const auto &playerPosition = registry.getComponent<Position>(playerEntity).p;
    const PlayerStats &playerStats = registry.getComponent<PlayerStats>(playerEntity);
    const auto &playerSprite = registry.getComponent<view::Sprite>(playerEntity).rect;

    const AnimationDirection attackDirection =
        input.mouseGrid.x >= playerPosition.x ? AnimationDirection::Right : AnimationDirection::Left;

    const float animationDuration = applyAnimation(registry, config, playerEntity, AnimationState::Attack,
                                                   playerStats.characterType, attackDirection);
    const SoundComponent sound = {config.playerClasses.melee.sounds.attack};

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

    const geometry::Vec2<float> hitBoxOffset = {
        .x = attackDirection == AnimationDirection::Right ? playerSprite.size.x / 2 : -playerSprite.size.x / 2,
        .y = -(attackProfile.meleeArc.reach) * playerStats.attackRange};

    const HitBox meleeHitBox{
        .offset = hitBoxOffset,
        .size = attackProfile.meleeArc.hitBoxSize + 2 * attackProfile.meleeArc.reach * playerStats.attackRange,
    };

    // add melee attack entity with all components
    // component references may be invalid: retrieve again from registry if used after this point
    const Entity meleeAttackEntity = registry.createEntity();
    registry.addComponent<Damage>(meleeAttackEntity, damageComponent);
    registry.addComponent<DamageTag>(meleeAttackEntity, {});
    registry.addComponent<Position>(meleeAttackEntity, damagePosition);
    registry.addComponent<HitBox>(meleeAttackEntity, meleeHitBox);
    registry.addComponent<SoundComponent>(playerEntity, sound);
    registry.addComponent<PlayerAttackTag>(meleeAttackEntity, {}); // Mark as player's attack for collision detection
}

void InputSystem::attackRanged(Registry &registry, const config::GameConfig &config, Entity playerEntity,
                               const controller::InputState &input, const config::AttackProfileConfig &attackProfile)
{
    const auto playerPosition = registry.getComponent<Position>(playerEntity).p;
    const PlayerStats &playerStats = registry.getComponent<PlayerStats>(playerEntity);
    const auto &playerSprite = registry.getComponent<view::Sprite>(playerEntity).rect;

    const AnimationDirection attackDirection =
        input.mouseGrid.x >= playerPosition.x ? AnimationDirection::Right : AnimationDirection::Left;

    applyAnimation(registry, config, playerEntity, AnimationState::Attack, playerStats.characterType, attackDirection);

    // add projectile
    const SoundComponent sound = {config.playerClasses.ranged.sounds.attack};
    const config::AnimationFrame projectileFrame = config::AnimationConfigHelper::getProjectileAnimationFrame(
        config, attackProfile.projectile, AnimationState::Idle, AnimationDirection::None, 0);
    const config::SpriteConfig &projectileSpriteConfig = projectileFrame.spriteConfig;

    const float projectileOffsetX =
        attackDirection == AnimationDirection::Right ? playerSprite.size.x : -projectileSpriteConfig.texture.size.x;
    const geometry::Vec2<float> projectileLaunchPosition{.x = playerPosition.x + projectileOffsetX,
                                                         .y = playerPosition.y + (playerSprite.size.y / 2)
                                                              - (projectileSpriteConfig.texture.size.y / 2)};

    const float projectileLaunchAngle =
        std::atan2(input.mouseGrid.y - projectileLaunchPosition.y, input.mouseGrid.x - projectileLaunchPosition.x);
    const Velocity projectileLaunchVelocity{
        {.x = playerStats.speedOfAttack * attackProfile.projectile.velocityScale * std::cos(projectileLaunchAngle),
         .y = playerStats.speedOfAttack * attackProfile.projectile.velocityScale * std::sin(projectileLaunchAngle)}};

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

    const view::Sprite projectileSprite{
        .rect = {projectileLaunchPosition, projectileSpriteConfig.texture.size},
        .imagePath = projectileSpriteConfig.texture.path,
    };

    const HitBox projectileHitBox{projectileSpriteConfig.hitBox.offset, projectileSpriteConfig.hitBox.size};

    // add projectile entity with all components
    // component references may be invalid: retrieve again from registry if used after this point
    const Entity projectileEntity = registry.createEntity();
    registry.addComponent<SoundComponent>(playerEntity, sound);
    registry.addComponent<Damage>(projectileEntity, projectileDamage);
    registry.addComponent<view::Sprite>(projectileEntity, projectileSprite);
    registry.addComponent<Position>(projectileEntity, {projectileLaunchPosition});
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
    auto &playerVelocity = registry.getComponent<Velocity>(playerEntity).v;

    const config::AnimationFrame currentFrame =
        config::AnimationConfigHelper::getPlayerAnimationFrame(config, playerStats.characterType, playerAnimation.state,
                                                               playerAnimation.direction, playerAnimation.currentFrame);

    playerVelocity *= currentFrame.moveSpeedMultiplier;
}

} // namespace game