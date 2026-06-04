#pragma once
#include "game/ecs/entity.hpp"
#include <set>

namespace game {

struct DamageTag {
    std::set<Entity> targets = {};
    std::set<Entity> targetsHit = {};
};

} // namespace game
