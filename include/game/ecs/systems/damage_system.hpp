#pragma once
#include "game/ecs/registry.hpp"

namespace game {

struct DamageSystem {
    void update(Registry &registry);
};

}