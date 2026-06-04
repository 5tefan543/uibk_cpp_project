#include "game/ecs/systems/input_system.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/location_table.hpp"

namespace game {

void InputSystem::update(Registry &registry, const controller::InputState &input)
{
    for (auto entity : registry.view<Velocity, PlayerStats>()) {
        Velocity &velocity = registry.getComponent<Velocity>(entity);
        PlayerStats &playerStats = registry.getComponent<PlayerStats>(entity);

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
        v.setLenght(playerStats.moveSpeed);
        velocity.x = v.x;
        velocity.y = v.y; // TODO: into Vec2
    }
}

} // namespace game