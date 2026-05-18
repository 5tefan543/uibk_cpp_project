#pragma once

#include "game/ecs/registry.hpp"
#include "game/ecs/systems/collision_detection_system.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/projectile.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/map.hpp"

namespace game {

void CollisionDetectionSystem::update(Registry &registry) {
    
    auto enemies = registry.view<EnemyTag, HitBox>();
    auto enemyProjectiles = registry.view<Projectile, EnemyTag, HitBox>();
    auto playerProjectiles = registry.view<Projectile, PlayerTag, HitBox>();
    auto playerHitBoxes = registry.view<PlayerTag, HitBox>();
    auto mapHitBoxes = registry.view<Map, HitBox>();

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

            if (enemyHitBox.rect.intersects(projectileHitBox.rect)) {
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
    