#include "game/ecs/systems/collision_detection_system.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/enemy_attack_tag.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/player_attack_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/registry.hpp"
#include "view/sprite.hpp"

#include <iostream>

namespace game {

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

    const Position &positionA = registry.getComponent<Position>(entityA);
    const HitBox &hitBoxA = registry.getComponent<HitBox>(entityA);

    const Position &positionB = registry.getComponent<Position>(entityB);
    const HitBox &hitBoxB = registry.getComponent<HitBox>(entityB);

    const Rectangle<float> hitBoxRectA{.position = {positionA.x + hitBoxA.offset.x, positionA.y + hitBoxA.offset.y},
                                       .size = {hitBoxA.size.x, hitBoxA.size.y}};

    const Rectangle<float> hitBoxRectB{.position = {positionB.x + hitBoxB.offset.x, positionB.y + hitBoxB.offset.y},
                                       .size = {hitBoxB.size.x, hitBoxB.size.y}};

    return hitBoxRectA.intersects(hitBoxRectB);
}

// Maybe move into own system
void CollisionDetectionSystem::activateDamage(const Entity &source, const Entity &target, Registry &registry)
{

    if (!registry.hasComponent<Damage>(source) || registry.hasComponent<Damage>(target)) {
        return; // Damage instances do not interact with each other
    }

    if (registry.hasComponent<PlayerAttackTag>(source) && registry.hasComponent<EnemyTag>(target)) {
        addTarget(source, target, registry);
        return;
    } else if (registry.hasComponent<EnemyAttackTag>(source) && registry.hasComponent<PlayerTag>(target)) {
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

    if (registry.hasComponent<MapTag>(entity)) {
        return; // Map is static and does not need boundary enforcement
    }

    if (registry.hasComponent<Damage>(entity)) {
        return;
    }

    Position &position = registry.getComponent<Position>(entity);
    HitBox &hitBox = registry.getComponent<HitBox>(entity);

    // Assuming there's only one map entity with a MapTag
    auto mapEntities = registry.view<MapTag, HitBox>();
    if (mapEntities.empty()) {
        return; // No map entity found
    }

    Entity mapEntity = mapEntities.front();
    const Position &mapPosition = registry.getComponent<Position>(mapEntity);
    const HitBox &mapHitBox = registry.getComponent<HitBox>(mapEntity);

    Rectangle<float> mapHitBoxRect{.position = {mapPosition.x + mapHitBox.offset.x, mapPosition.y + mapHitBox.offset.y},
                                   .size = {mapHitBox.size.x, mapHitBox.size.y}};

    Rectangle<float> entityHitBoxRect{{position.x + hitBox.offset.x, position.y + hitBox.offset.y},
                                      {hitBox.size.x, hitBox.size.y}};
    entityHitBoxRect.snapBack(mapHitBoxRect);

    position.x = entityHitBoxRect.position.x - hitBox.offset.x;
    position.y = entityHitBoxRect.position.y - hitBox.offset.y;
}

void CollisionDetectionSystem::update(Registry &registry)
{
    const std::vector<Entity> &entitiesWithHitBoxes = registry.view<HitBox, Position>();

    for (size_t i = 0; i < entitiesWithHitBoxes.size(); ++i) {
        enforceMapBound(entitiesWithHitBoxes.at(i), registry);
        if (registry.hasComponent<MapTag>(entitiesWithHitBoxes.at(i))) {
            continue; // Skip map for collision detection
        }
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