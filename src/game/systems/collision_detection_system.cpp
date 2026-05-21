#include "game/ecs/systems/collision_detection_system.hpp"
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/projectile.hpp"
#include "game/ecs/registry.hpp"
#include "view/sprite.hpp"

namespace game {

void CollisionDetectionSystem::updateHitBoxPosition(const Entity &entity, Registry &registry)
{
    if (!registry.hasComponent<Position>(entity) || !registry.hasComponent<HitBox>(entity)) {
        return;
    }

    HitBox &hitBox = registry.getComponent<HitBox>(entity);
    Position &position = registry.getComponent<Position>(entity);

    hitBox.rect.x = position.x;
    hitBox.rect.y = position.y;
    hitBox.isActive = true;
}

bool CollisionDetectionSystem::checkCollision(const Entity &entityA, const Entity &entityB, Registry &registry)
{
    const HitBox &hitBoxA = registry.getComponent<HitBox>(entityA);
    const HitBox &hitBoxB = registry.getComponent<HitBox>(entityB);

    if (!hitBoxA.isActive || !hitBoxB.isActive) {
        return false;
    }

    return hitBoxA.rect.intersects(hitBoxB.rect);
}

void CollisionDetectionSystem::initializeHitBoxes(Registry &registry)
{
    auto entitiesWithHitBoxes = registry.view<view::Sprite, Position>();
    for (size_t i = 0; i < entitiesWithHitBoxes.size(); ++i) {

        view::Sprite &sprite = registry.getComponent<view::Sprite>(entitiesWithHitBoxes[i]);
        // Problem with previous idea via alpha:
        // - Performance heavy to read pixel data for every sprite on initialization,
        // But hitbox values can be cached
        // Dependency to sf or other rendering library would be needed here
        // Or Renderer will be dependend on that system both no-gos
        // maybe make an external python script that takes pngs and gives back json hitboxes

        Position &position = registry.getComponent<Position>(entitiesWithHitBoxes[i]);
        HitBox &hitBox = registry.addComponent<HitBox>(
            entitiesWithHitBoxes[i], {
                                         .rect = {position.x, position.y, sprite.width, sprite.height},
                                         .isActive = true,
                                     });
    }
}

// Maybe move into own system
void CollisionDetectionSystem::applyDamage(const Entity &source, const Entity &target, Registry &registry) {}

void CollisionDetectionSystem::enforceMapBound(const Entity &entity, Registry &registry)
{
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

void CollisionDetectionSystem::update(Registry &registry)
{
    if (!isInitialized_) {
        initializeHitBoxes(registry);
        isInitialized_ = true;
        return;
    }

    const std::vector<Entity> &entitiesWithHitBoxes = registry.view<HitBox, Position>();

    for (size_t i = 0; i < entitiesWithHitBoxes.size(); ++i) {
        updateHitBoxPosition(entitiesWithHitBoxes.at(i), registry);
        enforceMapBound(entitiesWithHitBoxes.at(i), registry);

        for (size_t j = i + 1; j < entitiesWithHitBoxes.size(); ++j) {
            Entity entityA = entitiesWithHitBoxes[i];
            Entity entityB = entitiesWithHitBoxes[j];

            if (checkCollision(entityA, entityB, registry)) {
                // Handle collision between entityA and entityB
                // For example, you can check their tags and apply damage or other effects
                bool isEntityAPlayer = registry.hasComponent<PlayerTag>(entityA);
                bool isEntityBPlayer = registry.hasComponent<PlayerTag>(entityB);
                bool isEntityAEnemy = registry.hasComponent<EnemyTag>(entityA);
                bool isEntityBEnemy = registry.hasComponent<EnemyTag>(entityB);
                bool isEntityAProjectile = registry.hasComponent<Projectile>(entityA);
                bool isEntityBProjectile = registry.hasComponent<Projectile>(entityB);

                // Example collision handling logic
                if ((isEntityAPlayer && isEntityBEnemy) || (isEntityAEnemy && isEntityBPlayer)) {
                }

                if ((isEntityAProjectile && isEntityBEnemy) || (isEntityAEnemy && isEntityBProjectile)) {
                }
            }
        }
    }
}

} // namespace game