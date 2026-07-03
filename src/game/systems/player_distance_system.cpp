#include "game/ecs/systems/player_distance_system.hpp"
#include "game/ecs/components/distance_to_player.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include <limits>

namespace game {

void PlayerDistanceSystem::update(Registry &registry)
{
    const auto players = registry.view<PlayerTag, Position>();

    if (players.empty()) {
        for (const Entity enemy : registry.view<EnemyTag>()) {
            if (!registry.hasComponent<DistanceToPlayer>(enemy)) {
                registry.addComponent<DistanceToPlayer>(enemy, {});
            }

            DistanceToPlayer &distance = registry.getComponent<DistanceToPlayer>(enemy);
            distance.value = std::numeric_limits<float>::infinity();
            distance.hasPlayer = false;
        }
        return;
    }

    const Position &playerPosition = registry.getComponent<Position>(players.front());

    for (const Entity enemy : registry.view<EnemyTag, Position>()) {
        if (!registry.hasComponent<DistanceToPlayer>(enemy)) {
            registry.addComponent<DistanceToPlayer>(enemy, {});
        }

        const Position &enemyPosition = registry.getComponent<Position>(enemy);
        DistanceToPlayer &distance = registry.getComponent<DistanceToPlayer>(enemy);
        distance.value = (playerPosition.p - enemyPosition.p).length();
        distance.hasPlayer = true;
    }
}

} // namespace game