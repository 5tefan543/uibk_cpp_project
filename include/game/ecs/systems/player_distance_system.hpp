#pragma once

#include "game/ecs/registry.hpp"

namespace game {

struct PlayerDistanceSystem {
    void update(Registry &registry);
};

} // namespace game