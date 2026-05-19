#pragma once

#include "game/ecs/systems/collision_detection_system.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/map.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/projectile.hpp"
#include "game/ecs/registry.hpp"

namespace game {

bool CollisionDetectionSystem::checkCollision(const Entity &entityA, const Entity &entityB, Registry &registry)
{
    const HitBox &hitBoxA = registry.getComponent<HitBox>(entityA);
    const HitBox &hitBoxB = registry.getComponent<HitBox>(entityB);

    if (!hitBoxA.isActive || !hitBoxB.isActive) {
        return false;
    }

    return hitBoxA.rect.(hitBoxB.rect);
}

void CollisionDetectionSystem::update(Registry &registry)
{

    auto entitiesWithHitBoxes = registry.view<HitBox>();
    for (auto itA = entitiesWithHitBoxes.begin(); itA != entitiesWithHitBoxes.end(); ++itA) {
        Entity entityA = *itA;
        const HitBox &hitBoxA = registry.getComponent<HitBox>(entityA);
        std::vector<Entity> enemyHitBoxEntities;
        std::vector<Entity> playerProjectileHitBoxEntities;

        for (Entity enemy : enemies) {
            const HitBox &enemyHitBox = registry.getComponent<HitBox>(enemy);

            if (!enemyHitBox.isActive) {
                continue;
            }

            for (Entity playerProjectile : playerProjectiles) {
                const Projectile &projectile = registry.getComponent<Projectile>(playerProjectile);
                const HitBox &projectileHitBox = registry.getComponent<HitBox>(playerProjectile);

                if (!projectile.isActive || !projectileHitBox.isActive) {
                    continue;
                }

                if (checkCollision(enemy, playerProjectile, registry)) {
                    // Handle collision between enemy and player projectile
                    // For example, you might want to mark the projectile as inactive
                    // and apply damage to the enemy
                    registry.getComponent<Projectile>(playerProjectile).isActive = false;
                    // You would also want to apply damage to the enemy here
                }
            }
        }
    }
}
