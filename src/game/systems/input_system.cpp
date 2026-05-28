#include "game/ecs/systems/input_system.hpp"
#include "controller/persistence/persistence_manager.hpp"
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

void InputSystem::updateCooldown(float dt)
{
    timeSinceLastAttack_ += dt;
}

void InputSystem::attackRanged(Registry &registry, const PlayerStats &stats, Entity playerEntity,
                               const controller::InputState &input)
{

    if (timeSinceLastAttack_ <= 1.0f / stats.attackSpeed) {
        return; // Attack is still on cooldown
    }

    timeSinceLastAttack_ = 0.0f;
    Entity attackEntity = registry.createEntity();
    Position playerPosition = registry.getComponent<Position>(playerEntity);
    Damage damageComponent{
        .amount = 10.0f,
        .isColliding = false,
        .isMultiHit = false,
        .pushBackForce = 0.0f,
        .stunChance = 0.0f,
        .kind = DamageKind::Projectile,
        .params = ProjectileDamage{
            .speed = stats.speedOfAttack, .maxRange = stats.attackRange, .distanceTraveled = 0.0f, .targetsHit = 0}};

    Position position{.x = playerPosition.x, .y = playerPosition.y};

    view::Sprite sprite{.x = playerPosition.x,
                        .y = playerPosition.y,
                        .imagePath = controller::PersistenceManager::getConfig().assetConfig.projectilePath,
                        .width = 16.0f,
                        .height = 16.0f};
    HitBox hitBox{.rect = {position.x, position.y, sprite.width, sprite.height}, .isActive = true};
    float angle = std::atan2(input.mouseGridY - playerPosition.y, input.mouseGridX - playerPosition.x);
    Velocity velocity{.dx = stats.speedOfAttack * 10 * std::cos(angle),
                      .dy = stats.speedOfAttack * 10 * std::sin(angle)};
    registry.addComponent<Damage>(attackEntity, damageComponent);
    registry.addComponent<view::Sprite>(attackEntity, sprite);
    registry.addComponent<Position>(attackEntity, position);
    registry.addComponent<Velocity>(attackEntity, velocity);
    registry.addComponent<HitBox>(attackEntity, hitBox);
    registry.addComponent<PlayerTag>(attackEntity, {}); // Mark as player's attack for collision detection
}

void InputSystem::attackMelee(Registry &registry, const PlayerStats &stats, Entity playerEntity,
                              const controller::InputState &input)
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

    Damage damageComponent{.amount = 10.0f,
                           .isColliding = false,
                           .isMultiHit = false,
                           .pushBackForce = 0.0f,
                           .stunChance = 0.0f,
                           .kind = DamageKind::MeleeArc,
                           .params = MeleeArcDamage{.arcAngleDeg = 90.0f,
                                                    .arcRadius = stats.attackRange,
                                                    .activeTimeSec = attackDurationSec + 0.1f,
                                                    .elapsedSec = 0.0f}};

    Position position{.x = playerPosition.x, .y = playerPosition.y};

    HitBox hitBox{.rect = {position.x, position.y, 64, 64}, .isActive = true};

    registry.addComponent<Damage>(attackEntity, damageComponent);
    registry.addComponent<Position>(attackEntity, position);
    registry.addComponent<HitBox>(attackEntity, hitBox);
    registry.addComponent<PlayerTag>(attackEntity, {}); // Mark as player's attack for collision detection
}

void InputSystem::update(Registry &registry, const controller::InputState &input, float dt)
{
    updateCooldown(dt);

    for (auto entity : registry.view<Velocity, PlayerStats>()) {
        Velocity &velocity = registry.getComponent<Velocity>(entity);
        PlayerStats &playerStats = registry.getComponent<PlayerStats>(entity);

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
            if (playerStats.dmgKind == DamageKind::MeleeArc) {
                attackMelee(registry, playerStats, entity, input);
            } else {
                attackRanged(registry, playerStats, entity, input);
            }
        }

        if (input.mouseRightPressed) {
            // special abilities for ranged and melee here
            if (playerStats.dmgKind == DamageKind::MeleeArc) {
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