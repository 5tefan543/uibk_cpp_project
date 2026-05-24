#include "game/ecs/systems/damage_system.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/stats.hpp"

namespace game {

void DamageSystem::update(Registry &registry, float dt)
{
    auto damageEntities = registry.view<Damage, DamageTag>();
    for (size_t i = 0; i < damageEntities.size(); ++i) {
        Entity damageEntity = damageEntities[i];
        Damage &damage = registry.getComponent<Damage>(damageEntity);
        DamageTag &damageTag = registry.getComponent<DamageTag>(damageEntity);

        if (!damage.isColliding) {
            continue; // Skip if damage is not currently colliding
        }

        Entity targetEntity = damageTag.target;

        if (!registry.isEntityAlive(targetEntity)) {
            registry.removeComponent<DamageTag>(damageEntity);
            continue; // Skip if target entity is no longer alive, and remove the damage tag to find new target
        }

        if (registry.hasComponent<PlayerStats>(targetEntity)) {
            PlayerStats &playerStats = registry.getComponent<PlayerStats>(targetEntity);
            playerStats.health -= damage.amount;
            if (playerStats.health <= 0.0f) {
                registry.destroyEntity(targetEntity);
            }
        } else if (registry.hasComponent<EnemyStats>(targetEntity)) {
            EnemyStats &enemyStats = registry.getComponent<EnemyStats>(targetEntity);
            enemyStats.health -= damage.amount;
            if (enemyStats.health <= 0.0f) {
                registry.destroyEntity(targetEntity);
            }
        }

        // After applying damage, remove the damage entity
        registry.destroyEntity(damageEntity);
    }
}

} // namespace game