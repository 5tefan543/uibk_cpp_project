#include "game/ecs/systems/enemy_ai.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/sprite.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/location_table.hpp"
#include <cmath>
#include <iostream>

namespace game {

void EnemyAI::update(Registry &registry, LocationTable &locationTable)
{
    const auto players = registry.view<Velocity, PlayerTag>();
    if (players.size() > 0) {
        const auto player = players[0];
        const Position playerPos = registry.getComponent<Position>(player);
        const Vec2 posP{playerPos.x, playerPos.y};

        for (auto enemy : registry.view<Velocity, Sprite, Position, EnemyTag>()) {
            EnemyTag &enemyTag = registry.getComponent<EnemyTag>(enemy);
            Velocity &velocity = registry.getComponent<Velocity>(enemy);
            Position &enemyPos = registry.getComponent<Position>(enemy);
            Vec2 posE{enemyPos.x, enemyPos.y};

            // Set movement direction exactly towards player
            Vec2 v = posP - posE;

            // Calc. repelling force between enemies
            auto enemiesInRange = locationTable.getEntitiesInRange(posE, 50, registry);
            Vec2<float> repelOffset = {0, 0};
            for (auto [e, position] : enemiesInRange) {
                if (e == enemy) {
                    continue;
                }
                const auto p = Vec2{position.x, position.y}; // TODO: into Vec2
                const auto pToOther = (posE - p);
                repelOffset += pToOther / std::pow(pToOther.length(), 2); // increase repelling with proximity
                repelOffset.normalize();
            }
            v.normalize();
            v += repelOffset;

            v.setLenght(enemyTag.moveSpeed);
            velocity.dx = v.x; // TODO: into Vec2
            velocity.dy = v.y; // TODO: into Vec2
        }
    }
}

} // namespace game