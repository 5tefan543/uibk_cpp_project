#pragma once
#include "game/ecs/registry.hpp"

namespace game {

class DamageSystem {
  public:
    DamageSystem() = default;
    void update(Registry &registry);
};

} // namespace game