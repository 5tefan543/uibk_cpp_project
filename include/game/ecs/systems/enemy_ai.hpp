#pragma once

#include "game/ecs/registry.hpp"
#include "game/location_table.hpp"
#include "view/grid.hpp"

namespace game {
struct EnemyAI {
    void update(Registry &registry, locTab &locationTable);
};

} // namespace game