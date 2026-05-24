#pragma once
#include "game/ecs/registry.hpp"

namespace game {

class DamageSystem {
  private:
    void updateProjectile(Entity projectile, Registry &registry);

  public:
    DamageSystem() = default;
    void update(Registry &registry, float dt);
};

} // namespace game