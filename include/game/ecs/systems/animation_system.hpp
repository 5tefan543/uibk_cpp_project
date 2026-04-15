#pragma once

#include "game/ecs/registry.hpp"

namespace game {

class AnimationSystem {
  public:
    void update(Registry &registry, float dt);
};

} // namespace game
