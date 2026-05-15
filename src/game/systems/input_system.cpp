#include "game/ecs/systems/input_system.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/velocity.hpp"
#include <cmath>

namespace game {

void InputSystem::update(Registry &registry, const controller::InputState &input)
{
    for (auto entity : registry.view<Velocity, PlayerTag>()) {
        PlayerTag &playerTag = registry.getComponent<PlayerTag>(entity);
        Velocity &velocity = registry.getComponent<Velocity>(entity);

        velocity.dx = 0.0F;
        velocity.dy = 0.0F;

        // TODO: normalize diag. movement
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

    auto players = registry.view<Velocity, PlayerTag>();
    if (players.size() > 0) {
        auto player = players[0];
        Position playerPos = registry.getComponent<Position>(player);

        for (auto enemy : registry.view<Velocity, EnemyTag>()) {
            EnemyTag &enemyTag = registry.getComponent<EnemyTag>(enemy);
            Velocity &velocity = registry.getComponent<Velocity>(enemy);
            Position &enimyPos = registry.getComponent<Position>(enemy);

            // TODO: normalize diag. movement
            velocity.dx = (playerPos.x - enimyPos.x);
            velocity.dy = (playerPos.y - enimyPos.y);
            auto svdx = std::signbit(velocity.dx);
            auto svdy = std::signbit(velocity.dy);
            velocity.dx *= (enemyTag.moveSpeed / velocity.dx) * (svdx ? -1 : 1);
            velocity.dy *= (enemyTag.moveSpeed / velocity.dy) * (svdy ? -1 : 1);
            // std::sqrt(dX * dX + dY * dY)

            // dx² + dy² = vel² sqrt(dx² + dy²) = velScale dx, y *= vel / velScale
        }
    }
}

} // namespace game