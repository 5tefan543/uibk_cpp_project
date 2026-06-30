#pragma once

#include "game/ecs/registry.hpp"

namespace game {

class HealthBarSystem {
  public:
    HealthBarSystem() = default;
    void update(Registry &registry, float dt);
};

} // namespace game
