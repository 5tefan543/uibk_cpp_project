#pragma once

#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"

namespace game {

struct PersistedGame {
    int wave = 0;
    Position position;
    PlayerStats playerStats;
    bool shouldOpenStore = false;
};

} // namespace game