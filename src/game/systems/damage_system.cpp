#include "game/ecs/systems/damage_system.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"

#include <cmath>

namespace game {

void DamageSystem::update(Registry &registry, float dt)
{
    auto damageEntities = registry.view<Damage, DamageTag>();
    for (Entity damageEntity : damageEntities) {
        if (!registry.isEntityAlive(damageEntity)) {
            continue;
        }

        Damage &damage = registry.getComponent<Damage>(damageEntity);
        DamageTag &damageTag = registry.getComponent<DamageTag>(damageEntity);
        bool shouldDestroyDamage = false;

        if (damage.kind == DamageKind::Projectile) {
            if (auto *projectile = std::get_if<ProjectileDamage>(&damage.params)) {
                float distanceThisFrame = projectile->speed * dt;
                if (registry.hasComponent<Velocity>(damageEntity)) {
                    const auto &velocity = registry.getComponent<Velocity>(damageEntity).v;
                    distanceThisFrame = velocity.length() * dt;
                }
                projectile->distanceTraveled += distanceThisFrame;

                if (projectile->distanceTraveled >= projectile->maxRange) {
                    shouldDestroyDamage = true;
                }
            }
        } else if (damage.kind == DamageKind::MeleeArc) {
            if (auto *melee = std::get_if<MeleeArcDamage>(&damage.params)) {
                melee->elapsedSec += dt;
                if (melee->elapsedSec >= melee->activeTimeSec) {
                    shouldDestroyDamage = true;
                }
            }
        } else if (damage.kind == DamageKind::Beam) {
            if (auto *beam = std::get_if<BeamDamage>(&damage.params)) {
                beam->elapsedSec += dt;
                if (beam->elapsedSec >= beam->activeTimeSec) {
                    shouldDestroyDamage = true;
                }
            }
        } else if (damage.kind == DamageKind::Area) {
            if (auto *area = std::get_if<AreaDamage>(&damage.params)) {
                area->elapsedSec += dt;
                if (area->elapsedSec >= area->activeTimeSec) {
                    shouldDestroyDamage = true;
                }
            }
        }

        const bool hadTaggedTargets = !damageTag.targets.empty();
        bool hasAliveTarget = false;
        const auto taggedTargets = damageTag.targets;

        for (Entity targetEntity : taggedTargets) {
            if (damageTag.targetsHit.contains(targetEntity)) {
                continue; // Already hit
            }

            if (!registry.isEntityAlive(targetEntity)) {
                damageTag.targets.erase(targetEntity);
                continue; // Skip if target entity is no longer alive, and remove the damage tag to find new target
            }

            hasAliveTarget = true;

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
                    auto players = registry.view<PlayerStats>();
                    if (!players.empty()) {
                        PlayerStats &playerStats = registry.getComponent<PlayerStats>(players.front());
                        playerStats.score += enemyStats.scoreReward;
                        playerStats.currency += enemyStats.scoreReward;
                    }
                    registry.destroyEntity(targetEntity);
                }
            }

            damageTag.targetsHit.insert(targetEntity);

            if (damage.kind == DamageKind::Projectile) {
                if (auto *projectile = std::get_if<ProjectileDamage>(&damage.params)) {
                    if (projectile->maxTargets <= 0
                        || static_cast<std::size_t>(projectile->maxTargets) <= damageTag.targetsHit.size()) {
                        shouldDestroyDamage = true;
                    }
                }
            }

            if (shouldDestroyDamage) {
                break;
            }
        }

        if (damage.kind == DamageKind::Projectile && hadTaggedTargets && !hasAliveTarget) {
            shouldDestroyDamage = true;
        }

        if (shouldDestroyDamage) {
            registry.destroyEntity(damageEntity);
        }
    }
}

} // namespace game