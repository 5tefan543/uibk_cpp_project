#include "game/ecs/systems/input_system.hpp"
#include "config/animation_config_helper.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/player_attack_cooldown.hpp"
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
    auto players =
        registry.view<PlayerTag, Velocity, PlayerStats, view::Sprite, Position, Animation, PlayerAttackCooldown>();

    if (players.empty()) {
        return;
    }

    if (players.size() > 1) {
        logger::log(logger::LogLevel::WARNING,
                    "Multiple player entities found in the registry. Only the first one will be processed.");
    }

    Entity playerEntity = players.front();

    updateCooldown(registry, playerEntity, dtSec);
    updatePlayerVelocity(registry, playerEntity, input);
    updatePlayerAnimationState(registry, playerEntity, dtSec);
    handleAttack(registry, config, playerEntity, input);
    applyAnimationMoveSpeedModifier(registry, config, playerEntity);
}

void InputSystem::updateCooldown(Registry &registry, Entity entity, float dtSec)
{
    PlayerAttackCooldown &cooldown = registry.getComponent<PlayerAttackCooldown>(entity);
    cooldown.attackRemainingSec = std::max(0.0f, cooldown.attackRemainingSec - dtSec);
    cooldown.specialAttackRemainingSec = std::max(0.0f, cooldown.specialAttackRemainingSec - dtSec);
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
    const PlayerStats &stats = registry.getComponent<PlayerStats>(playerEntity);
    PlayerAttackCooldown &cooldown = registry.getComponent<PlayerAttackCooldown>(playerEntity);

    cooldown.attackDurationSec = 1.0f / stats.attackSpeed;
    cooldown.specialAttackDurationSec = 1.0f / stats.specialAttackSpeed;

    if (registry.hasComponent<Animation>(playerEntity)) {
        const Animation &animation = registry.getComponent<Animation>(playerEntity);
        if (animation.stateTimeRemaining > 0.0f && animation.state == AnimationState::Attack) {
            // if the player is in the middle of an attack animation, we don't want to interrupt it
            return;
        }
    }

    const config::AttackProfileConfig &attackProfile = config.playerClasses.getByType(stats.characterType).attack;

    if (input.mouseLeftPressed) {

        if (cooldown.attackRemainingSec > 0.0f) {
            return;
        }

        if (stats.characterType == CharacterType::Melee) {
            attackMelee(registry, config, playerEntity, input, attackProfile, false);
            cooldown.attackRemainingSec = cooldown.attackDurationSec;
        } else if (stats.characterType == CharacterType::Ranged) {
            attackRanged(registry, config, playerEntity, input, attackProfile, false);
            cooldown.attackRemainingSec = cooldown.attackDurationSec;
        }

        return;
    }

    if (input.mouseRightPressed) {

        if (cooldown.specialAttackRemainingSec > 0.0f) {
            return;
        }

        if (stats.characterType == CharacterType::Melee) {
            attackMelee(registry, config, playerEntity, input, attackProfile, true);
            cooldown.specialAttackRemainingSec = cooldown.specialAttackDurationSec;
        } else if (stats.characterType == CharacterType::Ranged) {
            attackRanged(registry, config, playerEntity, input, attackProfile, true);
            cooldown.specialAttackRemainingSec = cooldown.specialAttackDurationSec;
        }

        return;
    }
}

void InputSystem::attackMelee(Registry &registry, const config::GameConfig &config, Entity playerEntity,
                              const controller::InputState &input, const config::AttackProfileConfig &attackProfile,
                              bool specialAttack)
{
    const auto &playerPosition = registry.getComponent<Position>(playerEntity).p;
    const PlayerStats &playerStats = registry.getComponent<PlayerStats>(playerEntity);
    const auto &playerSprite = registry.getComponent<view::Sprite>(playerEntity).rect;

    const AnimationDirection attackDirection =
        input.mouseGrid.x >= playerPosition.x ? AnimationDirection::Right : AnimationDirection::Left;

    const float animationDuration = applyAnimation(registry, config, playerEntity, AnimationState::Attack,
                                                   playerStats.characterType, attackDirection);

    // add melee attack object: either a melee arc or a area attack
    Damage meleeAttackObjDamage;
    meleeAttackObjDamage.pushBackForce = attackProfile.pushBackForce;
    meleeAttackObjDamage.stunChance = attackProfile.stunChance;

    if (specialAttack) {
        meleeAttackObjDamage.amount = -0.95; // on each damage tick reduce the damage by 95% of the enemy's max health
        meleeAttackObjDamage.kind = DamageKind::Area;
        meleeAttackObjDamage.params = AreaDamage{.radius = attackProfile.area.radius,
                                                 .activeTimeSec = attackProfile.area.activeTimeSec,
                                                 .elapsedSec = 0.0f,
                                                 .telegraphTimeSec = attackProfile.area.telegraphTimeSec,
                                                 .initialHit = attackProfile.area.initialHit,
                                                 .damageTicks = attackProfile.area.damageTicks,
                                                 .elapsedSecSinceLastTick = 0.0f};
    } else {
        // Treat attackProfile.amount as a multiplier applied to the attacker's `attackPower`.
        meleeAttackObjDamage.amount = playerStats.attackPower * attackProfile.amount;
        meleeAttackObjDamage.kind = DamageKind::MeleeArc;
        meleeAttackObjDamage.params =
            MeleeArcDamage{.reach = attackProfile.meleeArc.reach,
                           .activeTimeSec = animationDuration + attackProfile.meleeArc.activeTimePaddingSec,
                           .elapsedSec = 0.0f};
    }

    const Position damagePosition{playerPosition.x, playerPosition.y};
    HitBox meleeAttackObjHitBox;
    std::optional<view::Sprite> meleeAttackObjSprite = std::nullopt;
    std::optional<Animation> meleeAttackObjAnimation = std::nullopt;

    if (specialAttack) {
        const config::AnimationFrame meleeObjFrame = config::AnimationConfigHelper::getAreaAnimationFrame(
            config, attackProfile.area, AnimationState::Idle, AnimationDirection::None, 0);

        const config::SpriteConfig &meleeAttackObjSpriteConfig = meleeObjFrame.spriteConfig;
        meleeAttackObjHitBox = {meleeAttackObjSpriteConfig.hitBox.offset, meleeAttackObjSpriteConfig.hitBox.size};
        meleeAttackObjSprite = view::Sprite{};
        meleeAttackObjSprite.value().rect.size = meleeAttackObjSpriteConfig.texture.size;
        meleeAttackObjSprite.value().imagePath = meleeAttackObjSpriteConfig.texture.path;
        meleeAttackObjAnimation = Animation{.state = AnimationState::Idle, .direction = AnimationDirection::None};
    } else {
        const geometry::Vec2<float> hitBoxOffset = {
            .x = attackDirection == AnimationDirection::Right ? playerSprite.size.x / 2 : -playerSprite.size.x / 2,
            .y = -(attackProfile.meleeArc.reach) * playerStats.attackRange};

        meleeAttackObjHitBox = {
            .offset = hitBoxOffset,
            .size = attackProfile.meleeArc.hitBoxSize + 2 * attackProfile.meleeArc.reach * playerStats.attackRange,
        };
    }

    const Sound sound = {specialAttack ? config.playerClasses.melee.sounds.special
                                       : config.playerClasses.melee.sounds.attack};

    // add melee attack entity with all components
    // component references may be invalid: retrieve again from registry if used after this point
    const Entity meleeAttackEntity = registry.createEntity();
    registry.addComponent<Damage>(meleeAttackEntity, meleeAttackObjDamage);
    registry.addComponent<DamageTag>(meleeAttackEntity, {});
    registry.addComponent<Position>(meleeAttackEntity, damagePosition);
    registry.addComponent<HitBox>(meleeAttackEntity, meleeAttackObjHitBox);
    registry.addComponent<Sound>(playerEntity, sound);
    registry.addComponent<PlayerAttackTag>(
        meleeAttackEntity, {playerStats.characterType}); // Mark as player's attack for collision detection

    if (meleeAttackObjSprite.has_value() && meleeAttackObjAnimation.has_value()) {
        registry.addComponent<view::Sprite>(meleeAttackEntity, meleeAttackObjSprite.value());
        registry.addComponent<Animation>(meleeAttackEntity, meleeAttackObjAnimation.value());
    }
}

void InputSystem::attackRanged(Registry &registry, const config::GameConfig &config, Entity playerEntity,
                               const controller::InputState &input, const config::AttackProfileConfig &attackProfile,
                               bool specialAttack)
{
    const auto playerPosition = registry.getComponent<Position>(playerEntity).p;
    const PlayerStats &playerStats = registry.getComponent<PlayerStats>(playerEntity);
    const auto &playerSprite = registry.getComponent<view::Sprite>(playerEntity).rect;

    const AnimationDirection attackDirection =
        input.mouseGrid.x >= playerPosition.x ? AnimationDirection::Right : AnimationDirection::Left;

    applyAnimation(registry, config, playerEntity, AnimationState::Attack, playerStats.characterType, attackDirection);

    // add ranged object: either a projectile or a invincible unicorn
    Sound sound = {specialAttack ? config.playerClasses.ranged.sounds.special
                                 : config.playerClasses.ranged.sounds.attack};

    const config::AnimationFrame rangedObjFrame =
        specialAttack ? config::AnimationConfigHelper::getUnicornAnimationFrame(
                            config, attackProfile.unicorn, AnimationState::Walk, attackDirection, 0)
                      : config::AnimationConfigHelper::getProjectileAnimationFrame(
                            config, attackProfile.projectile, AnimationState::Idle, AnimationDirection::None, 0);
    const config::SpriteConfig &rangedObjSpriteConfig = rangedObjFrame.spriteConfig;

    std::optional<Animation> rangedObjAnimation = std::nullopt;
    if (specialAttack) {
        rangedObjAnimation = Animation{.state = AnimationState::Walk, .direction = attackDirection};
    }

    const float rangedObjOffsetX =
        attackDirection == AnimationDirection::Right ? playerSprite.size.x : -rangedObjSpriteConfig.texture.size.x;
    const geometry::Vec2<float> rangedObjLaunchPosition{.x = playerPosition.x + rangedObjOffsetX,
                                                        .y = playerPosition.y + (playerSprite.size.y / 2)
                                                             - (rangedObjSpriteConfig.texture.size.y / 2)};

    const float rangedObjLaunchAngle =
        std::atan2(input.mouseGrid.y - rangedObjLaunchPosition.y, input.mouseGrid.x - rangedObjLaunchPosition.x);

    float velocityScale = specialAttack ? attackProfile.unicorn.velocityScale : attackProfile.projectile.velocityScale;
    const Velocity rangedObjLaunchVelocity{
        {.x = playerStats.speedOfAttack * velocityScale * std::cos(rangedObjLaunchAngle),
         .y = playerStats.speedOfAttack * velocityScale * std::sin(rangedObjLaunchAngle)}};

    Damage rangedObjDamage;

    rangedObjDamage.pushBackForce = attackProfile.pushBackForce;
    rangedObjDamage.stunChance = attackProfile.stunChance;

    if (specialAttack) {
        rangedObjDamage.amount = -1; // kills all enemies in its path
        rangedObjDamage.kind = DamageKind::Unicorn;
        rangedObjDamage.params = UnicornDamage{.speed = playerStats.speedOfAttack};
    } else {
        // Treat attackProfile.amount as a multiplier applied to the attacker's `attackPower`.
        rangedObjDamage.amount = playerStats.attackPower * attackProfile.amount;
        rangedObjDamage.kind = DamageKind::Projectile;
        rangedObjDamage.params = ProjectileDamage{.speed = playerStats.speedOfAttack,
                                                  .maxRange = playerStats.attackRange,
                                                  .distanceTraveled = 0.0f,
                                                  .maxTargets = 1};
    }

    const view::Sprite rangedObjSprite{
        .rect = {rangedObjLaunchPosition, rangedObjSpriteConfig.texture.size},
        .imagePath = rangedObjSpriteConfig.texture.path,
    };

    const HitBox rangedObjHitBox{rangedObjSpriteConfig.hitBox.offset, rangedObjSpriteConfig.hitBox.size};

    // add rangedObj entity with all components
    // component references may be invalid: retrieve again from registry if used after this point
    const Entity rangedObjEntity = registry.createEntity();
    registry.addComponent<Sound>(playerEntity, sound);
    registry.addComponent<Damage>(rangedObjEntity, rangedObjDamage);
    registry.addComponent<view::Sprite>(rangedObjEntity, rangedObjSprite);
    registry.addComponent<Position>(rangedObjEntity, {rangedObjLaunchPosition});
    registry.addComponent<Velocity>(rangedObjEntity, rangedObjLaunchVelocity);
    registry.addComponent<HitBox>(rangedObjEntity, rangedObjHitBox);
    registry.addComponent<PlayerAttackTag>(
        rangedObjEntity, {playerStats.characterType}); // Mark as player's attack for collision detection
    registry.addComponent<DamageTag>(rangedObjEntity, {});

    if (rangedObjAnimation.has_value()) {
        registry.addComponent<Animation>(rangedObjEntity, rangedObjAnimation.value());
    }
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