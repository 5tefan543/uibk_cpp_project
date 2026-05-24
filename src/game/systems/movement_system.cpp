#include "game/ecs/systems/movement_system.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/velocity.hpp"

#include <cmath>
#include <iostream>
#include <variant>

namespace game {

void MovementSystem::update(Registry &registry, float dt)
{
    for (auto entity : registry.view<Position, Velocity>()) {
        Position &position = registry.getComponent<Position>(entity);
        Velocity &velocity = registry.getComponent<Velocity>(entity);

        if (velocity.dx == 0.0f && velocity.dy == 0.0f) {
            continue;
        }

        if (registry.hasComponent<Damage>(entity)) {
            Damage &damage = registry.getComponent<Damage>(entity);
            if (damage.kind == DamageKind::Projectile) {
                if (auto *projectile = std::get_if<ProjectileDamage>(&damage.params)) {
                    projectile->distanceTraveled +=
                        std::sqrt(velocity.dx * velocity.dx + velocity.dy * velocity.dy) * dt;
                    if (projectile->distanceTraveled >= projectile->maxRange) {
                        registry.destroyEntity(entity);
                        continue;
                    }
                }
            }
        }

        position.x += velocity.dx * dt;
        position.y += velocity.dy * dt;
    }
}

} // namespace game