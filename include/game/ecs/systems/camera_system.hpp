#pragma once

#include "game/ecs/registry.hpp"

namespace game {

class CameraSystem {
  public:
    void update(Registry &registry);
};

} // namespace game
