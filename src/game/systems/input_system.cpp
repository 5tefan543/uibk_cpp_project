#include "game/ecs/systems/input_system.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/velocity.hpp"

namespace game {

void InputSystem::update(Registry &registry, const controller::InputState &input)
{
    for (auto entity : registry.view<Velocity, PlayerTag>()) {
        PlayerTag &playerTag = registry.getComponent<PlayerTag>(entity);
        Velocity &velocity = registry.getComponent<Velocity>(entity);

        velocity.dx = 0.0F;
        velocity.dy = 0.0F;

        if (input.leftHeld) {
            velocity.dx -= playerTag.moveSpeed;
        }
        if (input.rightHeld) {
            velocity.dx += playerTag.moveSpeed;
        }
        if (input.upHeld) {
            velocity.dy -= playerTag.moveSpeed;
        }
        if (input.downHeld) {
            velocity.dy += playerTag.moveSpeed;
        }
    }
}

} // namespace game