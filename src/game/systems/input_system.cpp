#include "game/ecs/systems/input_system.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include <cmath>

namespace game {

void InputSystem::update(Registry &registry, const controller::InputState &input)
{
    for (auto entity : registry.view<Velocity, PlayerStats>()) {
        Velocity &velocity = registry.getComponent<Velocity>(entity);
        PlayerStats &playerStats = registry.getComponent<PlayerStats>(entity);

        velocity.dx = 0.0F;
        velocity.dy = 0.0F;

        // TODO: normalize diag. movement
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