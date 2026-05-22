#include "game/ecs/systems/input_system.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"

namespace game {

void InputSystem::updateCooldown(float dt)
{
    timeSinceLastAttack_ += dt;
}

void InputSystem::attack(Registry &registry, const PlayerStats &stats, Entity playerEntity)
{

    if (timeSinceLastAttack_ <= 1.0f / stats.attackSpeed) {
        return; // Attack is still on cooldown
    }

    timeSinceLastAttack_ = 0.0f;
    Entity attackEntity = registry.createEntity();
    registry.addComponent<DamageTag>(attackEntity, {});

    Damage damageComponent{.amount = 10.0f,
                           .isColliding = false,
                           .kind = DamageKind::Projectile,
                           .params = ProjectileDamage{.speed = stats.speedOfAttack,
                                                      .maxRange = stats.attackRange,
                                                      .distanceTraveled = 0.0f,
                                                      .pushbackForce = 0.0f,
                                                      .targetsHit = 0}};

    registry.addComponent<Damage>(attackEntity, damageComponent);
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
            attack(registry, playerStats, entity);
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