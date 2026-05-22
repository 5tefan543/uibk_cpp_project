#pragma once

#include "game/ecs/registry.hpp"

namespace game {

class CollisionDetectionSystem {
  private:
    bool isInitialized_;
    int wave_;
    bool checkCollision(const Entity &entityA, const Entity &entityB, Registry &registry);
    void updateHitBoxPosition(const Entity &entity, Registry &registry);
    void enforceMapBound(const Entity &entity, Registry &registry);
    void applyDamage(const Entity &source, const Entity &target, Registry &registry);
    void initializeHitBoxes(Registry &registry);

  public:
    CollisionDetectionSystem() : isInitialized_(false), wave_(0) {}
    void update(Registry &registry, int wave);
};

} // namespace game