#pragma once
#include "game/ecs/components/stats.hpp"
#include "game/ecs/registry.hpp"

namespace game {

class DamageSystem {
  private:
    void updateProjectile(Registry &registry, float dt);
    void updateMelee(Registry &registry, float dt);
    void updateBeam(Registry &registry, float dt);
    void updateArea(Registry &registry, float dt);

  public:
    DamageSystem() = default;
    void update(Registry &registry, float dt);
};

} // namespace game