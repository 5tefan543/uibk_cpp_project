#include "game/ecs/systems/input_system.hpp"
#include "config/asset_manager.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/location_table.hpp"
#include "view/sprite.hpp"
#include <cmath>
#include <iostream>

namespace game {

const config::AttackProfileConfig &InputSystem::getAttackProfile(const config::GameConfig &config,
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
                               const controller::InputState &input, const config::AttackProfileConfig &attackProfile,
                               const config::GameConfig &config)
{

    if (timeSinceLastAttack_ <= 1.0f / stats.attackSpeed) {
        return; // Attack is still on cooldown
    }

    const view::Sprite playerSprite = registry.hasComponent<view::Sprite>(playerEntity)
                                          ? registry.getComponent<view::Sprite>(playerEntity)
                                          : view::Sprite{};

    timeSinceLastAttack_ = 0.0f;
    Entity projectileEntity = registry.createEntity();
    Position playerPosition = registry.getComponent<Position>(playerEntity);
    Damage damageComponent{
        .amount = attackProfile.amount,
        .pushBackForce = attackProfile.pushBackForce,
        .stunChance = attackProfile.stunChance,
        .kind = attackProfile.kind,
        .params = ProjectileDamage{
            .speed = stats.speedOfAttack, .maxRange = stats.attackRange, .distanceTraveled = 0.0f, .maxTargets = 1}};

    const config::AnimationFrame projectileAnimationFrame = config::AssetManager::getProjectileAnimationFrame(
        config, attackProfile.projectile, AnimationState::Idle, AnimationDirection::None, 0);
    const config::SpriteConfig &projectileSpriteConfig = projectileAnimationFrame.spriteConfig;

    AnimationDirection attackDirection =
        input.mouseGridX >= playerPosition.x ? AnimationDirection::Right : AnimationDirection::Left;
    float offsetX =
        attackDirection == AnimationDirection::Right ? playerSprite.width : -projectileSpriteConfig.texture.size.x;
    Position position{.x = playerPosition.x + offsetX,
                      .y = playerPosition.y + (playerSprite.height / 2) - (projectileSpriteConfig.texture.size.y / 2)};
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
                        .imagePath = projectileSpriteConfig.texture.path,
                        .width = projectileSpriteConfig.texture.size.x,
                        .height = projectileSpriteConfig.texture.size.y};

    // TODO: define hitbox from projectileSpriteConfig.hitBox instead of using the whole sprite as hitbox
    HitBox hitBox{.rect = {sprite.x, sprite.y, sprite.width, sprite.height}};

    float angle = std::atan2(input.mouseGridY - position.y, input.mouseGridX - position.x);
    Velocity velocity{.x = stats.speedOfAttack * attackProfile.projectile.velocityScale * std::cos(angle),
                      .y = stats.speedOfAttack * attackProfile.projectile.velocityScale * std::sin(angle)};

    registry.addComponent<Damage>(projectileEntity, damageComponent);
    registry.addComponent<view::Sprite>(projectileEntity, sprite);
    registry.addComponent<Position>(projectileEntity, position);
    registry.addComponent<Velocity>(projectileEntity, velocity);
    registry.addComponent<HitBox>(projectileEntity, hitBox);
    registry.addComponent<PlayerTag>(projectileEntity, {});
    registry.addComponent<DamageTag>(projectileEntity, {});
}

void InputSystem::attackMelee(Registry &registry, const PlayerStats &stats, Entity playerEntity,
                              const controller::InputState &input, const config::AttackProfileConfig &attackProfile)
{

    if (timeSinceLastAttack_ <= 1.0f / stats.attackSpeed) {
        return; // Attack is still on cooldown
    }
    const view::Sprite playerSprite = registry.hasComponent<view::Sprite>(playerEntity)
                                          ? registry.getComponent<view::Sprite>(playerEntity)
                                          : view::Sprite{};
    Position playerPosition = registry.getComponent<Position>(playerEntity);
    AnimationDirection attackDirection =
        input.mouseGridX >= playerPosition.x ? AnimationDirection::Right : AnimationDirection::Left;

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
    Entity meleeAttackEntity = registry.createEntity();
    float offsetX = attackDirection == AnimationDirection::Right ? playerSprite.width / 2 : -playerSprite.width / 2;
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

    registry.addComponent<Damage>(meleeAttackEntity, damageComponent);
    registry.addComponent<DamageTag>(meleeAttackEntity, damageTag);
    registry.addComponent<Position>(meleeAttackEntity, position);
    registry.addComponent<HitBox>(meleeAttackEntity, hitBox);
    registry.addComponent<PlayerTag>(meleeAttackEntity, {}); // Mark as player's attack for collision detection
}

void InputSystem::update(Registry &registry, const config::GameConfig &config, const controller::InputState &input,
                         float dt)
{
    updateCooldown(dt);

    for (auto entity : registry.view<Velocity, PlayerStats>()) {
        Velocity &velocity = registry.getComponent<Velocity>(entity);
        PlayerStats &playerStats = registry.getComponent<PlayerStats>(entity);
        const config::AttackProfileConfig &attackProfile = getAttackProfile(config, playerStats.characterType);

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
                velocity.x *= animation.attackMoveSpeedMultiplier;
                velocity.y *= animation.attackMoveSpeedMultiplier;
            }
        }
    }
}

} // namespace game