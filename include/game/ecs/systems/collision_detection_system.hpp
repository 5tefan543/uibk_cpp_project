#pragma once

#include "game/ecs/registry.hpp"

namespace game {

class CollisionDetectionSystem {
  private:
    bool checkCollision(const Entity &entityA, const Entity &entityB, Registry &registry);

  public:
    void update(Registry &registry);
};

} // namespace game