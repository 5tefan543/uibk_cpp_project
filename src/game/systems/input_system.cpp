#include "game/ecs/systems/input_system.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
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
}

void InputSystem::attackRanged(Registry &registry, const PlayerStats &stats, Entity playerEntity,
                               const controller::InputState &input,
                               const controller::AttackProfileConfig &attackProfile,
                               const controller::GameConfig &config)
{

    if (timeSinceLastAttack_ <= 1.0f / stats.attackSpeed) {
        return; // Attack is still on cooldown
    }

    timeSinceLastAttack_ = 0.0f;
    Entity attackEntity = registry.createEntity();
    Position playerPosition = registry.getComponent<Position>(playerEntity);
    Damage damageComponent{
        .amount = attackProfile.amount,
        .pushBackForce = attackProfile.pushBackForce,
        .stunChance = attackProfile.stunChance,
        .kind = attackProfile.kind,
        .params = ProjectileDamage{
            .speed = stats.speedOfAttack, .maxRange = stats.attackRange, .distanceTraveled = 0.0f, .targetsHit = 0}};

    Position position{.x = playerPosition.x, .y = playerPosition.y};

    view::Sprite sprite{.x = playerPosition.x,
                        .y = playerPosition.y,
                        .imagePath = config.assetConfig.projectilePath,
                        .width = attackProfile.projectile.spriteWidth,
                        .height = attackProfile.projectile.spriteHeight};
    HitBox hitBox{.rect = {position.x, position.y, sprite.width, sprite.height}};
    float angle = std::atan2(input.mouseGridY - playerPosition.y, input.mouseGridX - playerPosition.x);
    Velocity velocity{.dx = stats.speedOfAttack * attackProfile.projectile.velocityScale * std::cos(angle),
                      .dy = stats.speedOfAttack * attackProfile.projectile.velocityScale * std::sin(angle)};
    registry.addComponent<Damage>(attackEntity, damageComponent);
    registry.addComponent<view::Sprite>(attackEntity, sprite);
    registry.addComponent<Position>(attackEntity, position);
    registry.addComponent<Velocity>(attackEntity, velocity);
    registry.addComponent<HitBox>(attackEntity, hitBox);
    registry.addComponent<PlayerTag>(attackEntity, {}); // Mark as player's attack for collision detection
}

void InputSystem::attackMelee(Registry &registry, const PlayerStats &stats, Entity playerEntity,
                              const controller::InputState &input, const controller::AttackProfileConfig &attackProfile)
{

    if (timeSinceLastAttack_ <= 1.0f / stats.attackSpeed) {
        return; // Attack is still on cooldown
    }

    Position playerPosition = registry.getComponent<Position>(playerEntity);
    Direction attackDirection = input.mouseGridX >= playerPosition.x ? Direction::Right : Direction::Left;

    float attackDurationSec = 0.65f;
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

    Damage damageComponent{
        .amount = attackProfile.amount,
        .pushBackForce = attackProfile.pushBackForce,
        .stunChance = attackProfile.stunChance,
        .kind = attackProfile.kind,
        .params = MeleeArcDamage{.reach = attackProfile.meleeArc.reach,
                                 .activeTimeSec = attackDurationSec + attackProfile.meleeArc.activeTimePaddingSec,
                                 .elapsedSec = 0.0f}};

    Position position{.x = playerPosition.x, .y = playerPosition.y};

    HitBox hitBox{
        .rect = {position.x, position.y, attackProfile.meleeArc.hitBoxWidth, attackProfile.meleeArc.hitBoxHeight}};

    registry.addComponent<Damage>(attackEntity, damageComponent);
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

        velocity.dx = 0.0F;
        velocity.dy = 0.0F;

        if (input.leftHeld) {
            velocity.dx -= playerStats.moveSpeed;
        }
        if (input.rightHeld) {
            velocity.dx += playerStats.moveSpeed;
        }
        if (input.upHeld) {
            velocity.dy -= playerStats.moveSpeed;
        }
        if (input.downHeld) {
            velocity.dy += playerStats.moveSpeed;
        }

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