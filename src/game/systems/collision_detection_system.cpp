#include "game/ecs/systems/collision_detection_system.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/registry.hpp"
#include "view/sprite.hpp"

#include <iostream>

namespace game {

void CollisionDetectionSystem::updateHitBoxPosition(const Entity &entity, Registry &registry)
{
    if (registry.hasComponent<MapTag>(entity)) {
        return; // Map is static and does not need hitbox position updates
    }
    if (!registry.hasComponent<Position>(entity) || !registry.hasComponent<HitBox>(entity)) {
        return;
    }

    HitBox &hitBox = registry.getComponent<HitBox>(entity);
    Position &position = registry.getComponent<Position>(entity);

    hitBox.rect.x = position.x;
    hitBox.rect.y = position.y;
}

bool CollisionDetectionSystem::checkCollision(const Entity &entityA, const Entity &entityB, Registry &registry)
{
    // collision pairs to skip checking:

    if (registry.hasComponent<PlayerTag>(entityA) && registry.hasComponent<PlayerTag>(entityB)) {
        return false; // Skip player-player collision
    }
    if (registry.hasComponent<EnemyTag>(entityA) && registry.hasComponent<EnemyTag>(entityB)) {
        return false; // Skip enemy-enemy collision
    }
    if (registry.hasComponent<Damage>(entityA) && registry.hasComponent<Damage>(entityB)) {
        return false; // Skip damage-damage collision
    }

    const HitBox &hitBoxA = registry.getComponent<HitBox>(entityA);
    const HitBox &hitBoxB = registry.getComponent<HitBox>(entityB);

    return hitBoxA.rect.intersects(hitBoxB.rect);
}

CollisionDetectionSystem::CollisionDetectionSystem() : isInitialized_(false), wave_(0) {}

void CollisionDetectionSystem::initializeHitBoxes(Registry &registry)
{
    auto entitiesWithHitBoxes = registry.view<view::Sprite, Position>();
    for (size_t i = 0; i < entitiesWithHitBoxes.size(); ++i) {
        if (registry.hasComponent<HitBox>(entitiesWithHitBoxes[i])) {
            continue; // Skip if hitbox already exists
        }

        view::Sprite &sprite = registry.getComponent<view::Sprite>(entitiesWithHitBoxes[i]);
        // Problem with previous idea via alpha:
        // - Performance heavy to read pixel data for every sprite on initialization,
        // But hitbox values can be cached
        // Dependency to sf or other rendering library would be needed here
        // Or Renderer will be dependend on that system both no-gos
        // maybe make an external python script that takes pngs and gives back json hitboxes

        Position &position = registry.getComponent<Position>(entitiesWithHitBoxes[i]);
        registry.addComponent<HitBox>(entitiesWithHitBoxes[i],
                                      {
                                          .rect = {position.x, position.y, sprite.width, sprite.height},
                                      });
    }
}

// Maybe move into own system
void CollisionDetectionSystem::activateDamage(const Entity &source, const Entity &target, Registry &registry)
{

    if (!registry.hasComponent<Damage>(source) || registry.hasComponent<Damage>(target)) {
        return; // Damage instances do not interact with each other
    }

    if (registry.hasComponent<PlayerTag>(source) && registry.hasComponent<EnemyStats>(target)) {
        addTarget(source, target, registry);
        return;
    } else if (registry.hasComponent<EnemyTag>(source) && registry.hasComponent<PlayerStats>(target)) {
        addTarget(source, target, registry);
        return;
    } else {
        return;
    }
}

void CollisionDetectionSystem::addTarget(const Entity &source, const Entity &target, Registry &registry)
{
    if (!registry.hasComponent<DamageTag>(source)) {
        DamageTag damage{};
        damage.targets.insert(target);
        registry.addComponent<DamageTag>(source, damage);
    } else {
        DamageTag &damage = registry.getComponent<DamageTag>(source);
        damage.targets.insert(target);
    }
}

void CollisionDetectionSystem::enforceMapBound(const Entity &entity, Registry &registry)
{

    if (registry.hasComponent<Damage>(entity)) {
        return;
    }

    if (!registry.hasComponent<Position>(entity) || !registry.hasComponent<HitBox>(entity)) {
        return;
    }

    Position &position = registry.getComponent<Position>(entity);
    HitBox &hitBox = registry.getComponent<HitBox>(entity);

    // Assuming there's only one map entity with a MapTag
    auto mapEntities = registry.view<MapTag, HitBox>();
    if (mapEntities.empty()) {
        return; // No map entity found
    }

    const HitBox &mapHitBox = registry.getComponent<HitBox>(mapEntities[0]);
    Rectangle<float> entityRect{position.x, position.y, hitBox.rect.width, hitBox.rect.height};
    entityRect.snapBack(mapHitBox.rect);

    position.x = entityRect.x;
    position.y = entityRect.y;
}

void CollisionDetectionSystem::update(Registry &registry, int wave)
{
    if (wave != wave_) {
        isInitialized_ = false;
    }
    if (!isInitialized_) {
        initializeHitBoxes(registry);
        wave_ = wave;
        isInitialized_ = true;
        return;
    }

    const std::vector<Entity> &entitiesWithHitBoxes = registry.view<HitBox, Position>();

    for (size_t i = 0; i < entitiesWithHitBoxes.size(); ++i) {
        enforceMapBound(entitiesWithHitBoxes.at(i), registry);
        if (registry.hasComponent<MapTag>(entitiesWithHitBoxes.at(i))) {
            continue; // Skip map for collision detection
        }
        updateHitBoxPosition(entitiesWithHitBoxes.at(i), registry);

        for (size_t j = 0; j < entitiesWithHitBoxes.size(); ++j) {
            Entity source = entitiesWithHitBoxes.at(i);
            Entity target = entitiesWithHitBoxes.at(j);
            if (checkCollision(source, target, registry)) {
                activateDamage(source, target, registry);
            }
        }
    }
}

} // namespace game