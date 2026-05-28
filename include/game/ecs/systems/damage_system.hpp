#pragma once
#include "game/ecs/components/stats.hpp"
#include "game/ecs/registry.hpp"

namespace game {

class DamageSystem {
  public:
    DamageSystem() = default;
    void update(Registry &registry, float dt);
};

} // namespace game