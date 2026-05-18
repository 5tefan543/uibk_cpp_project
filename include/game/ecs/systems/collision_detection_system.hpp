#pragma once

#include "game/ecs/registry.hpp"

namespace game {

struct  CollisionDetectionSystem {
    void update(Registry &registry);
};

}