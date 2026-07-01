#pragma once

#include "game/ecs/registry.hpp"

namespace game {

struct MovementSystem {
    void update(Registry &registry, float dtSec);
};

} // namespace game