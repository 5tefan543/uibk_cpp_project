#include "game/ecs/systems/input_system.hpp"

#include "game/ecs/components/velocity.hpp"

namespace game {

void InputSystem::update(Registry &registry, const controller::InputState &input)
{
    auto view = registry.view<Velocity>();

    for (auto entity : view) {
        auto &velocity = view.get<Velocity>(entity);

        velocity.dx = 0.0F;
        velocity.dy = 0.0F;

        constexpr float moveSpeed = 200.0F;

        if (input.left) {
            velocity.dx -= moveSpeed;
        }
        if (input.right) {
            velocity.dx += moveSpeed;
        }
        if (input.up) {
            velocity.dy -= moveSpeed;
        }
        if (input.down) {
            velocity.dy += moveSpeed;
        }
    }
}

} // namespace game