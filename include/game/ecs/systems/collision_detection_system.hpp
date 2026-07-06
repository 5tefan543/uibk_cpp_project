#pragma once

#include "game/ecs/registry.hpp"
#include "game/location_table.hpp"

namespace game {

class CollisionDetectionSystem {
  private:
    bool checkCollision(const Entity &entityA, const Entity &entityB, Registry &registry);
    bool checkProjectileCollision(const Entity &projectile, const Entity &target, Registry &registry,
                                  const float dtSec);
    void enforceMapBound(const Entity &entity, Registry &registry);
    void activateDamage(const Entity &source, const Entity &target, Registry &registry);
    void addTarget(const Entity &target, const Entity &source, Registry &registry);

  public:
    void update(Registry &registry, const LocationTable &locationTable, const float dtSec);
};

} // namespace game