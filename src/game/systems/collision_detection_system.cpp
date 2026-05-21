#include "game/ecs/systems/collision_detection_system.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/projectile.hpp"
#include "game/ecs/registry.hpp"
#include "view/sprite.hpp"

namespace game {

void CollisionDetectionSystem::updateHitBoxPositions(Registry &registry)
{
    auto entitiesWithHitBoxes = registry.view<HitBox, view::Sprite, Position>();
    for (size_t i = 0; i < entitiesWithHitBoxes.size(); ++i) {

        HitBox &hitBox = registry.getComponent<HitBox>(entitiesWithHitBoxes[i]);
        Position &position = registry.getComponent<Position>(entitiesWithHitBoxes[i]);

        hitBox.rect.x = position.x;
        hitBox.rect.y = position.y;
        hitBox.isActive = true;
    }
}

bool CollisionDetectionSystem::checkCollision(const Entity &entityA, const Entity &entityB, Registry &registry)
{
    const HitBox &hitBoxA = registry.getComponent<HitBox>(entityA);
    const HitBox &hitBoxB = registry.getComponent<HitBox>(entityB);

    if (!hitBoxA.isActive || !hitBoxB.isActive) {
        return false;
    }

    return hitBoxA.rect.findIntersection(hitBoxB.rect).has_value();
}

void CollisionDetectionSystem::initializeHitBoxes(Registry &registry)
{
    auto entitiesWithHitBoxes = registry.view<view::Sprite, Position>();
    for (size_t i = 0; i < entitiesWithHitBoxes.size(); ++i) {

        view::Sprite &sprite = registry.getComponent<view::Sprite>(entitiesWithHitBoxes[i]);
        Position &position = registry.getComponent<Position>(entitiesWithHitBoxes[i]);
        HitBox &hitBox = registry.addComponent<HitBox>(
            entitiesWithHitBoxes[i], {
                                         .rect = {position.x, position.y, sprite.width, sprite.height},
                                         .isActive = true,
                                     });
    }
}

void CollisionDetectionSystem::applyDamage(Entity &source, Entity &target, Registry &registry) {}

void CollisionDetectionSystem::forceMove(Entity &entity, Registry &registry) {}

void CollisionDetectionSystem::update(Registry &registry)
{
    if (!isInitialized_) {
        initializeHitBoxes(registry);
        isInitialized_ = true;
        return;
    }

    updateHitBoxPositions(registry);

    auto entitiesWithHitBoxes = registry.view<HitBox, view::Sprite, Position>();

    for (size_t i = 0; i < entitiesWithHitBoxes.size(); ++i) {

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