#include "game/ecs/systems/enemy_ai_system.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/location_table.hpp"
#include "view/sprite.hpp"
#include <cmath>

namespace game {

void EnemyAI::update(Registry &registry, LocationTable &locationTable)
{
    const auto players = registry.view<Velocity, PlayerStats>();
    if (players.empty()) {
        return;
    }
    const auto player = players.front();
    const Position playerPos = registry.getComponent<Position>(player);
    const Vec2 posP{playerPos.x, playerPos.y};

    for (auto enemy : registry.view<Velocity, view::Sprite, Position, EnemyTag, EnemyStats>()) {
        auto &[vx, vy] = registry.getComponent<Velocity>(enemy);
        auto &[px, py] = registry.getComponent<Position>(enemy);
        EnemyStats &enemyStats = registry.getComponent<EnemyStats>(enemy);
        Vec2 posE{px, py};

        // Set movement direction exactly towards player
        Vec2 v = posP - posE;

        // TODO: add player attack:
        // if (v.length() < 30) {attack_player();}

        // Prevent shooting over target (player) position
        if (v.length() < 5) {
            vx = 0; // TODO: into Vec2
            vy = 0; // TODO: into Vec2
            continue;
        }

        // Calc. repelling force between enemies
        auto enemiesInRange = locationTable.getEntitiesInRange(posE, 50, registry);
        Vec2<float> repelOffset = {0, 0};
        for (auto [e, position] : enemiesInRange) {
            if (e == enemy) {
                continue;
            }
            const auto p = Vec2{position.x, position.y}; // TODO: into Vec2
            const auto pToOther = (posE - p);
            repelOffset += pToOther / (std::pow(pToOther.length(), 1.5)); // increase repelling with proximity
        }

        // Divert straight forwards movement to player with repelling offset
        v.normalize();
        v += repelOffset;
        v *= enemyStats.moveSpeed;

        // Enforce enemy speed limit if repelling force would boost it over max
        auto l = std::min(v.length(), enemyStats.moveSpeed);

        // Prevents jittery movement in enemy heaps by
        // - decreasing speed inverse propotional to max speed
        // - and stopping movement below a certaing percentage
        l *= std::pow(l / enemyStats.moveSpeed, 2);
        if (l / enemyStats.moveSpeed < 0.02) {
            vx = 0; // TODO: into Vec2
            vy = 0; // TODO: into Vec2
            continue;
        }
        v.setLenght(l);

        vx = v.x; // TODO: into Vec2
        vy = v.y; // TODO: into Vec2
    }
}

} // namespace game