#pragma once

#include "game/ecs/registry.hpp"

namespace game {

class CollisionDetectionSystem {
  private:
    bool checkCollision(const Entity &entityA, const Entity &entityB, Registry &registry);
    void enforceMapBound(const Entity &entity, Registry &registry);
    void activateDamage(const Entity &source, const Entity &target, Registry &registry);
    void addTarget(const Entity &target, const Entity &source, Registry &registry);

  public:
    void update(Registry &registry);
};

} // namespace game