#include "game/ecs/systems/damage_system.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "logging/log.hpp"
#include <format>

#include <cmath>

namespace game {

namespace {

constexpr float hitAnimationDurationSec = 0.16f;

void triggerHitAnimation(Registry &registry, Entity targetEntity)
{
    if (!registry.hasComponent<Animation>(targetEntity)) {
        return;
    }

    Animation &animation = registry.getComponent<Animation>(targetEntity);
    startTimedAnimation(animation, AnimationState::Hit, animation.direction, hitAnimationDurationSec);
}

} // namespace

DamageInformation DamageSystem::updateProjectile(Registry &registry, Damage &damage, ProjectileDamage &projectile,
                                                 Entity damageEntity, float dtSec)
{
    float distanceThisFrame = projectile.speed * dtSec;
    DamageInformation amount = {.actualDamageAmount = damage.amount, .shouldBeRemoved = false};
    if (registry.hasComponent<Velocity>(damageEntity)) {
        const Velocity &velocity = registry.getComponent<Velocity>(damageEntity);
        distanceThisFrame = std::sqrt(velocity.v.x * velocity.v.x + velocity.v.y * velocity.v.y) * dtSec;
    }
    projectile.distanceTraveled += distanceThisFrame;

    if (projectile.distanceTraveled >= projectile.maxRange) {
        amount.shouldBeRemoved = true;
    }
    return amount;
}

DamageInformation DamageSystem::updateUnicorn(const Registry &registry, Entity entity, const Damage &damage)
{
    DamageInformation result;
    result.actualDamageAmount = damage.amount;
    result.shouldBeRemoved = false;

    auto mapEntities = registry.view<MapTag, HitBox>();

    if (mapEntities.empty()) {
        logger::log(logger::LogLevel::WARNING, "No map entity found in the registry.");
        return result;
    }

    if (registry.hasComponent<Position>(entity)) {
        const Position &unicornPosition = registry.getComponent<Position>(entity);
        const HitBox &unicornHitBox = registry.getComponent<HitBox>(entity);
        const Position &mapPosition = registry.getComponent<Position>(mapEntities.front());
        const HitBox &mapHitBox = registry.getComponent<HitBox>(mapEntities.front());

        const geometry::Rectangle<float> hitBoxRectUnicorn{unicornHitBox.offset + unicornPosition.p,
                                                           unicornHitBox.size};
        const geometry::Rectangle<float> hitBoxRectMap{mapHitBox.offset + mapPosition.p, mapHitBox.size};

        if (!hitBoxRectUnicorn.intersects(hitBoxRectMap)) {
            result.shouldBeRemoved = true;
        }
    }
    return result;
}

DamageInformation DamageSystem::updateMelee(Damage &damage, MeleeArcDamage &melee, float dtSec)
{
    DamageInformation result;
    result.actualDamageAmount = damage.amount;
    result.shouldBeRemoved = false;
    melee.elapsedSec += dtSec;
    if (melee.elapsedSec > melee.activeTimeSec)
        result.shouldBeRemoved = true;

    return result;
}

DamageInformation DamageSystem::updateBeam(Damage &damage, BeamDamage &beam, DamageTag &tag, float dtSec)
{
    DamageInformation result;
    result.actualDamageAmount = damage.amount / beam.damageTicks;
    result.shouldBeRemoved = false;
    beam.elapsedSec += dtSec;
    beam.elapsedSecSinceLastTick += dtSec;
    bool resetTargets = beam.elapsedSecSinceLastTick > beam.elapsedSec / beam.damageTicks;
    tag.targets = {};
    if (resetTargets) {
        tag.targetsHit = {};
        beam.elapsedSecSinceLastTick = 0.0f;
    }
    if (beam.elapsedSec > beam.activeTimeSec) {
        result.shouldBeRemoved = true;
    }
    return result;
}

DamageInformation DamageSystem::updateArea(Damage &damage, AreaDamage &area, DamageTag &tag, float dtSec)
{
    const int damageTickCount = std::max(1, area.damageTicks);
    const bool isTelegraphing = area.elapsedSec < area.telegraphTimeSec;

    DamageInformation result;
    if (damage.amount < 0.0f) {
        result.actualDamageAmount = damage.amount;
    } else if (isTelegraphing) {
        result.actualDamageAmount = damage.amount * area.initialHit;
    } else {
        result.actualDamageAmount = damage.amount * (1.0f - area.initialHit) / damageTickCount;
    }
    result.shouldBeRemoved = false;
    area.elapsedSec += dtSec;
    area.elapsedSecSinceLastTick += dtSec;
    const bool resetHitTargets = area.elapsedSecSinceLastTick > area.elapsedSec / damageTickCount;
    if (resetHitTargets) {
        tag.targetsHit = {};
        area.elapsedSecSinceLastTick = 0.0f;
    }
    if (area.elapsedSec > area.activeTimeSec) {
        result.shouldBeRemoved = true;
    }
    return result;
}

void DamageSystem::update(Registry &registry, float dtSec)
{
    // Apply health regeneration (health per second) before processing damage
    for (Entity e : registry.view<PlayerStats>()) {
        if (!registry.isEntityAlive(e))
            continue;
        PlayerStats &ps = registry.getComponent<PlayerStats>(e);
        if (ps.healthRegen > 0.0f && ps.health < ps.maxHealth) {
            ps.health = std::min(ps.maxHealth, ps.health + ps.healthRegen * dtSec);
        }
    }
    for (Entity e : registry.view<EnemyStats>()) {
        if (!registry.isEntityAlive(e))
            continue;
        EnemyStats &es = registry.getComponent<EnemyStats>(e);
        if (es.healthRegen > 0.0f && es.health < es.maxHealth) {
            es.health = std::min(es.maxHealth, es.health + es.healthRegen * dtSec);
        }
    }
    auto damageEntities = registry.view<Damage, DamageTag>();
    for (Entity damageEntity : damageEntities) {
        if (!registry.isEntityAlive(damageEntity)) {
            continue;
        }

        DamageInformation currentDamage;
        Damage &damage = registry.getComponent<Damage>(damageEntity);
        DamageTag &damageTag = registry.getComponent<DamageTag>(damageEntity);

        switch (damage.kind) {
        case DamageKind::MeleeArc:
            if (std::holds_alternative<MeleeArcDamage>(damage.params)) {
                auto &melee = std::get<MeleeArcDamage>(damage.params);
                currentDamage = updateMelee(damage, melee, dtSec);
            }
            break;
        case DamageKind::Projectile:
            if (std::holds_alternative<ProjectileDamage>(damage.params)) {
                auto &projectile = std::get<ProjectileDamage>(damage.params);
                currentDamage = updateProjectile(registry, damage, projectile, damageEntity, dtSec);
            }
            break;
        case DamageKind::Unicorn:
            if (std::holds_alternative<UnicornDamage>(damage.params)) {
                currentDamage = updateUnicorn(registry, damageEntity, damage);
            }
            break;
        case DamageKind::Beam:
            if (std::holds_alternative<BeamDamage>(damage.params)) {
                auto &beam = std::get<BeamDamage>(damage.params);
                currentDamage = updateBeam(damage, beam, damageTag, dtSec);
            }
            break;
        case DamageKind::Area:
            if (std::holds_alternative<AreaDamage>(damage.params)) {
                auto &area = std::get<AreaDamage>(damage.params);
                currentDamage = updateArea(damage, area, damageTag, dtSec);
            }
            break;
        default:
            logger::log(logger::LogLevel::ERROR, "unknown damageKind");
            break;
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
                continue; // Skip if target entity is no longer alive, and remove the damage tag to allow new target
            }

            hasAliveTarget = true;

            float damageAmount = currentDamage.actualDamageAmount;

            if (damageAmount == 0.0f) {
                continue;
            }

            if (registry.hasComponent<PlayerStats>(targetEntity)) {
                PlayerStats &playerStats = registry.getComponent<PlayerStats>(targetEntity);

                if (damageAmount < 0.0f) {
                    // Negative damage values represent percentage-based damage.
                    // Example: -0.25f means 25% of the player's max health.
                    damageAmount = playerStats.maxHealth * (-damageAmount);
                    // Percentage-based damage bypasses flat defense reduction.
                    playerStats.health -= damageAmount;
                } else {
                    // Flat damage is reduced by the target's defense (additive flat reduction).
                    const float effective = std::max(0.0f, damageAmount - playerStats.defense);
                    playerStats.health -= effective;
                }

                if (playerStats.health <= 0.0f) {
                    registry.destroyEntity(targetEntity);
                } else {
                    triggerHitAnimation(registry, targetEntity);
                }
            } else if (registry.hasComponent<EnemyStats>(targetEntity)) {
                EnemyStats &enemyStats = registry.getComponent<EnemyStats>(targetEntity);

                if (damageAmount < 0.0f) {
                    // Negative damage values represent percentage-based damage.
                    // Example: -0.25f means 25% of the enemy's max health.
                    damageAmount = enemyStats.maxHealth * (-damageAmount);
                    // Percentage-based damage bypasses flat defense reduction.
                    enemyStats.health -= damageAmount;
                } else {
                    // Flat damage is reduced by the target's defense (additive flat reduction).
                    const float effective = std::max(0.0f, damageAmount - enemyStats.defense);
                    enemyStats.health -= effective;
                }

                if (enemyStats.health <= 0.0f) {
                    auto players = registry.view<PlayerStats>();
                    if (!players.empty()) {
                        PlayerStats &playerStats = registry.getComponent<PlayerStats>(players.front());
                        playerStats.score += enemyStats.scoreReward;
                        playerStats.currency += enemyStats.scoreReward;
                    }
                    registry.destroyEntity(targetEntity);
                } else {
                    triggerHitAnimation(registry, targetEntity);
                }
            }

            damageTag.targetsHit.insert(targetEntity);

            // check if max targets are reached
            if (damage.kind == DamageKind::Projectile) {
                if (std::holds_alternative<ProjectileDamage>(damage.params)) {
                    const auto &projectile = std::get<ProjectileDamage>(damage.params);
                    if (projectile.maxTargets <= 0
                        || static_cast<std::size_t>(projectile.maxTargets) <= damageTag.targetsHit.size()) {
                        currentDamage.shouldBeRemoved = true;
                    }
                }
            }

            if (currentDamage.shouldBeRemoved) {
                break;
            }
        }

        switch (damage.kind) {
        case DamageKind::Area:
            damageTag.targets = {};
            break;
        case DamageKind::Beam:
            damageTag.targets = {};
            break;
        case DamageKind::Projectile:
            if (hadTaggedTargets && !hasAliveTarget) {
                currentDamage.shouldBeRemoved = true;
            }
            break;
        default:
            break;
        }

        if (currentDamage.shouldBeRemoved) {
            registry.destroyEntity(damageEntity);
        }
    }
}

} // namespace game