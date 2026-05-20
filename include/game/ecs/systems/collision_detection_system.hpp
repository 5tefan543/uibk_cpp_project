#pragma once

#include "game/ecs/registry.hpp"

namespace game {

class CollisionDetectionSystem {
  private:
    bool checkCollision(const Entity &entityA, const Entity &entityB, Registry &registry);
    void updateHitBoxPositions(Registry &registry);
    void forceMove(Entity &entity, Registry &registry);
    void applyDamage(Entity &source, Entity &target, Registry &registry);
    void initializeHitBoxes(Registry &registry);

    bool isInitialized_ = false;

  public:
    void update(Registry &registry);
};

} // namespace game