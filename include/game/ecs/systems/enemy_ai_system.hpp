#pragma once

#include "game/ecs/registry.hpp"
#include "game/location_table.hpp"

namespace game {

struct EnemyAI {
    void update(Registry &registry, LocationTable &locationTable);
};

} // namespace game