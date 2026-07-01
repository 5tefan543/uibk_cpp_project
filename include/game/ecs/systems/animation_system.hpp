#pragma once

#include "config/game_config.hpp"
#include "game/ecs/registry.hpp"

namespace game {

class AnimationSystem {
  public:
    void update(Registry &registry, const config::GameConfig &config, float dtSec);
};

} // namespace game
