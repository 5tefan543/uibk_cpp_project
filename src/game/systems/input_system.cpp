#include "game/ecs/systems/input_system.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/location_table.hpp"
#include "view/sprite.hpp"
#include <cmath>
#include <iostream>

namespace game {

const controller::AttackProfileConfig &InputSystem::getAttackProfile(const controller::GameConfig &config,
                                                                     CharacterType characterType) const
{
    if (characterType == CharacterType::Melee) {
        return config.playerClasses.melee.attack;
    }

    return config.playerClasses.ranged.attack;
}

void InputSystem::updateCooldown(float dt)
{
    timeSinceLastAttack_ += dt;
    timeSinceLastSpecialMove_ += dt;
    timeSinceLastDash_ += dt;
}

void InputSystem::attackRanged(Registry &registry, const PlayerStats &stats, Entity playerEntity,
                               const controller::InputState &input,
                               const controller::AttackProfileConfig &attackProfile,
                               const controller::GameConfig &config)
{

    if (timeSinceLastAttack_ <= 1.0f / stats.attackSpeed) {
        return; // Attack is still on cooldown
    }

    const view::Sprite playerSprite = registry.hasComponent<view::Sprite>(playerEntity)
                                          ? registry.getComponent<view::Sprite>(playerEntity)
                                          : view::Sprite{};

    timeSinceLastAttack_ = 0.0f;
    Entity attackEntity = registry.createEntity();
    Position playerPosition = registry.getComponent<Position>(playerEntity);
    Damage damageComponent{
        .amount = attackProfile.amount,
        .pushBackForce = attackProfile.pushBackForce,
        .stunChance = attackProfile.stunChance,
        .kind = attackProfile.kind,
        .params = ProjectileDamage{
            .speed = stats.speedOfAttack, .maxRange = stats.attackRange, .distanceTraveled = 0.0f, .maxTargets = 1}};

    Direction attackDirection = input.mouseGridX >= playerPosition.x ? Direction::Right : Direction::Left;
    float offsetX = attackDirection == Direction::Right ? playerSprite.width : -attackProfile.projectile.spriteWidth;
    Position position{.x = playerPosition.x + offsetX,
                      .y = playerPosition.y + (playerSprite.height / 2) - (attackProfile.projectile.spriteHeight / 2)};
    if (registry.hasComponent<Animation>(playerEntity)) {
        Animation &playerAnimation = registry.getComponent<Animation>(playerEntity);
        playerAnimation.overrideState = AnimationOverrideState::Attack;
        playerAnimation.overrideTimeRemaining =
            static_cast<float>(playerAnimation.attackTotalFrames) * playerAnimation.attackFrameDuration;
        playerAnimation.overrideDirection = attackDirection;
        playerAnimation.direction = attackDirection;
        playerAnimation.currentFrame = 0;
        playerAnimation.frameTimer = 0.0f;
    }

    view::Sprite sprite{.x = position.x,
                        .y = position.y,
                        .imagePath = config.assetConfig.projectilePath,
                        .width = attackProfile.projectile.spriteWidth,
                        .height = attackProfile.projectile.spriteHeight};
    HitBox hitBox{.rect = {sprite.x, sprite.y, sprite.width, sprite.height}};
    float angle = std::atan2(input.mouseGridY - position.y, input.mouseGridX - position.x);
    Velocity velocity{.dx = stats.speedOfAttack * attackProfile.projectile.velocityScale * std::cos(angle),
                      .dy = stats.speedOfAttack * attackProfile.projectile.velocityScale * std::sin(angle)};
    registry.addComponent<Damage>(attackEntity, damageComponent);
    registry.addComponent<view::Sprite>(attackEntity, sprite);
    registry.addComponent<Position>(attackEntity, position);
    registry.addComponent<Velocity>(attackEntity, velocity);
    registry.addComponent<HitBox>(attackEntity, hitBox);
    registry.addComponent<PlayerTag>(attackEntity, {});
    registry.addComponent<DamageTag>(attackEntity, {});
}

void InputSystem::attackMelee(Registry &registry, const PlayerStats &stats, Entity playerEntity,
                              const controller::InputState &input, const controller::AttackProfileConfig &attackProfile)
{

    if (timeSinceLastAttack_ <= 1.0f / stats.attackSpeed) {
        return; // Attack is still on cooldown
    }
    const view::Sprite playerSprite = registry.hasComponent<view::Sprite>(playerEntity)
                                          ? registry.getComponent<view::Sprite>(playerEntity)
                                          : view::Sprite{};
    Position playerPosition = registry.getComponent<Position>(playerEntity);
    Direction attackDirection = input.mouseGridX >= playerPosition.x ? Direction::Right : Direction::Left;

    float attackDurationSec = 0.32f;
    if (registry.hasComponent<Animation>(playerEntity)) {
        Animation &playerAnimation = registry.getComponent<Animation>(playerEntity);
        playerAnimation.overrideState = AnimationOverrideState::Attack;
        playerAnimation.overrideTimeRemaining =
            static_cast<float>(playerAnimation.attackTotalFrames) * playerAnimation.attackFrameDuration;
        playerAnimation.overrideDirection = attackDirection;
        playerAnimation.direction = attackDirection;
        playerAnimation.currentFrame = 0;
        playerAnimation.frameTimer = 0.0f;

        attackDurationSec = playerAnimation.overrideTimeRemaining;
    }

    timeSinceLastAttack_ = 0.0f;
    Entity attackEntity = registry.createEntity();
    float offsetX = attackDirection == Direction::Right ? playerSprite.width / 2 : -playerSprite.width / 2;
    Damage damageComponent{
        .amount = attackProfile.amount,
        .pushBackForce = attackProfile.pushBackForce,
        .stunChance = attackProfile.stunChance,
        .kind = attackProfile.kind,
        .params = MeleeArcDamage{.reach = attackProfile.meleeArc.reach,
                                 .activeTimeSec = attackDurationSec + attackProfile.meleeArc.activeTimePaddingSec,
                                 .elapsedSec = 0.0f}};

    Position position{.x = playerPosition.x + offsetX, .y = playerPosition.y};
    DamageTag damageTag{};

    HitBox hitBox{.rect = {position.x + offsetX, position.y - (attackProfile.meleeArc.reach) * stats.attackRange,
                           attackProfile.meleeArc.hitBoxWidth + 2 * attackProfile.meleeArc.reach * stats.attackRange,
                           attackProfile.meleeArc.hitBoxHeight + 2 * attackProfile.meleeArc.reach * stats.attackRange}};

    registry.addComponent<Damage>(attackEntity, damageComponent);
    registry.addComponent<DamageTag>(attackEntity, damageTag);
    registry.addComponent<Position>(attackEntity, position);
    registry.addComponent<HitBox>(attackEntity, hitBox);
    registry.addComponent<PlayerTag>(attackEntity, {}); // Mark as player's attack for collision detection
}

void InputSystem::update(Registry &registry, const controller::GameConfig &config, const controller::InputState &input,
                         float dt)
{
    updateCooldown(dt);

    for (auto entity : registry.view<Velocity, PlayerStats>()) {
        Velocity &velocity = registry.getComponent<Velocity>(entity);
        PlayerStats &playerStats = registry.getComponent<PlayerStats>(entity);
        const controller::AttackProfileConfig &attackProfile = getAttackProfile(config, playerStats.characterType);

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

        if (input.mouseLeftPressed) {
            if (attackProfile.kind == DamageKind::MeleeArc) {
                attackMelee(registry, playerStats, entity, input, attackProfile);
            } else if (attackProfile.kind == DamageKind::Projectile) {
                attackRanged(registry, playerStats, entity, input, attackProfile, config);
            }
        }

        if (input.mouseRightPressed) {
            // special abilities for ranged and melee here
            if (attackProfile.kind == DamageKind::MeleeArc) {
                // Area
            } else {
                // Beam or multi projectiles
            }
        }

        if (registry.hasComponent<Animation>(entity)) {
            const Animation &animation = registry.getComponent<Animation>(entity);
            if (animation.overrideState == AnimationOverrideState::Attack) {
                velocity.dx *= animation.attackMoveSpeedMultiplier;
                velocity.dy *= animation.attackMoveSpeedMultiplier;
            }
        }
    }
}

} // namespace game