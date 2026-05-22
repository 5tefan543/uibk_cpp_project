#pragma once

#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"

namespace controller {

struct PersistedGame {
    int wave = 0;
    int score = 0;
    int currency = 0;
    game::Position position;
    game::PlayerStats playerStats;
};

} // namespace controller