#include "game/ecs/systems/movement_system.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/velocity.hpp"

namespace game {

void MovementSystem::update(Registry &registry, float dt)
{
    for (auto entity : registry.view<Position, Velocity>()) {
        auto &position = registry.getComponent<Position>(entity).p;
        auto &velocity = registry.getComponent<Velocity>(entity).v;
        position += velocity * dt;
    }
}

} // namespace game