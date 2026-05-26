#pragma once
#include "game/ecs/components/stats.hpp"
#include "game/ecs/registry.hpp"

namespace game {

class DamageSystem {
  private:
    void updateDamage(Entity damageEntity, Stats &stats, Registry &registry);

  public:
    DamageSystem() = default;
    void update(Registry &registry, float dt);
};

} // namespace game