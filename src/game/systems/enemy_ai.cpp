#include "game/ecs/systems/enemy_ai.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/sprite.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/location_table.hpp"
#include <cmath>

namespace game {

template <typename T>
struct Vec2 {
    T x;
    T y;

    Vec2<T> operator+(Vec2<T> other) const { return Vec2{x + other.x, y + other.y}; }
    Vec2<T> operator-(Vec2<T> other) const { return Vec2{x - other.x, y - other.y}; }
    Vec2<T> operator*(T scalar) const { return Vec2{x * scalar, y * scalar}; }
    Vec2<T> operator/(Vec2<T> other) const { return Vec2{x / other.x, y / other.y}; }
    Vec2<T> operator/(T scalar) const { return Vec2{x / scalar, y / scalar}; }
    Vec2<T> operator*(Vec2<T> other) const { return Vec2{x * other.x, y * other.y}; }
    void operator+=(T scalar)
    {
        x += scalar;
        y += scalar;
    };
    void operator-=(T scalar)
    {
        x -= scalar;
        y -= scalar;
    };
    void operator*=(T scalar)
    {
        x *= scalar;
        y *= scalar;
    };
    void operator/=(T scalar)
    {
        x /= scalar;
        y /= scalar;
    };
    void operator+=(Vec2<T> other)
    {
        x += other.x;
        y += other.y;
    };
    void operator-=(Vec2<T> other)
    {
        x -= other.x;
        y -= other.y;
    };
    void operator*=(Vec2<T> other)
    {
        x *= other.x;
        y *= other.y;
    };
    void operator/=(Vec2<T> other)
    {
        x /= other.x;
        y /= other.y;
    };
    T length() const { return std::sqrt(x * x + y * y); };
    Vec2<T> abs() const { return {std::abs(x), std::abs(y)}; };
};

// Ideas: change direction with delay/(de)accel coupled to dt
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

            Vec2 v = posP - posE;
            v *= enemyTag.moveSpeed / v.length();

            float radiusNear = 100;
            auto enemiesNear = locationTable.getEntitiesNear(posE.x, posE.y, radiusNear);
            for (Entity e : enemiesNear) {
                if (e == enemy) {
                    continue;
                }
                Position &posNear = registry.getComponent<Position>(e);
                auto towardsother = posE - Vec2{posNear.x, posNear.y};
                if (towardsother.length() < radiusNear) {
                    towardsother *= (enemyTag.moveSpeed / towardsother.length());
                    v += towardsother;
                    // std::cout << "enemy " << enemy << "moved away" << std::endl;
                }
            }

            v *= enemyTag.moveSpeed / v.length();
            velocity.dx = v.x;
            velocity.dy = v.y;
        }
    }
}

} // namespace game