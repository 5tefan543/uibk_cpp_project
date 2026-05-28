#include "game/ecs/systems/movement_system.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/velocity.hpp"

namespace game {

void MovementSystem::update(Registry &registry, float dt)
{
    for (auto entity : registry.view<Position, Velocity>()) {
        Position &position = registry.getComponent<Position>(entity);
        Velocity &velocity = registry.getComponent<Velocity>(entity);

        if (velocity.dx == 0.0f && velocity.dy == 0.0f) {
            continue;
        }

        position.x += velocity.dx * dt;
        position.y += velocity.dy * dt;
    }
}

} // namespace game