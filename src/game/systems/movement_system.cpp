#include "game/ecs/systems/movement_system.hpp"

#include "game/ecs/components/position.hpp"
#include "game/ecs/components/velocity.hpp"

#include <iostream>

namespace game {

void MovementSystem::update(Registry &registry, float dt)
{
    for (auto entity : registry.view<Position, Velocity>()) {
        Position &position = registry.getComponent<Position>(entity);
        Velocity &velocity = registry.getComponent<Velocity>(entity);

        Position before = position;

        position.x += velocity.dx * dt;
        position.y += velocity.dy * dt;

        if (position.x != before.x || position.y != before.y) {
            std::cout << "Entity " << entity << " moved to (" << position.x << ", " << position.y << ")\n";
        }
    }
}

} // namespace game