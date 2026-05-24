#include "game/ecs/systems/input_system.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/player_tag.hpp"
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

void InputSystem::attack(Registry &registry, const PlayerStats &stats, Entity playerEntity,
                         const controller::InputState &input)
{

    if (timeSinceLastAttack_ <= 1.0f / stats.attackSpeed) {
        return; // Attack is still on cooldown
    }

    timeSinceLastAttack_ = 0.0f;
    Entity attackEntity = registry.createEntity();
    registry.addComponent<DamageTag>(attackEntity, {});
    Position playerPosition = registry.getComponent<Position>(playerEntity);
    Damage damageComponent{.amount = 10.0f,
                           .isColliding = false,
                           .kind = DamageKind::Projectile,
                           .params = ProjectileDamage{.speed = stats.speedOfAttack,
                                                      .maxRange = stats.attackRange,
                                                      .distanceTraveled = 0.0f,
                                                      .pushbackForce = 0.0f,
                                                      .targetsHit = 0}};

    Position position{.x = playerPosition.x, .y = playerPosition.y};

    view::Sprite sprite{.x = playerPosition.x,
                        .y = playerPosition.y,
                        .imagePath = controller::PersistenceManager::getConfig().assetConfig.projectilePath,
                        .width = 16.0f,
                        .height = 16.0f};
    HitBox hitBox{.rect = {position.x, position.y, sprite.width, sprite.height}, .isActive = true};
    float angle = std::atan2(input.mouseGridY - playerPosition.y, input.mouseGridX - playerPosition.x);
    Velocity velocity{.dx = stats.speedOfAttack * std::cos(angle), .dy = stats.speedOfAttack * std::sin(angle)};
    std::cout << "SpeedOfAttack: " << stats.attackSpeed << " attacks/sec\n";
    std::cout << "angle: " << angle << " radians\n";
    std::cout << "Player attacked with velocity (" << stats.speedOfAttack * std::cos(angle) << ", " << velocity.dy
              << ")\n";
    registry.addComponent<Damage>(attackEntity, damageComponent);
    registry.addComponent<view::Sprite>(attackEntity, sprite);
    registry.addComponent<Position>(attackEntity, position);
    registry.addComponent<Velocity>(attackEntity, velocity);
    registry.addComponent<HitBox>(attackEntity, hitBox);
}

void InputSystem::update(Registry &registry, const controller::InputState &input, float dt)
{
    updateCooldown(dt);

    for (auto entity : registry.view<Velocity, PlayerStats>()) {
        Velocity &velocity = registry.getComponent<Velocity>(entity);
        PlayerStats &playerStats = registry.getComponent<PlayerStats>(entity);

        velocity.dx = 0.0F;
        velocity.dy = 0.0F;

        if (input.mouseLeftPressed) {
            attack(registry, playerStats, entity, input);
        }

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
    }
}

} // namespace game