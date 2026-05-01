#pragma once

namespace controller {

// only temporary struct
// should be replaced by actual components and systems that manage them
struct PlayerStats {
    int maxHealth = 100;
    int attackPower = 10;
    int defense = 5;
};

struct PersistedGame {
    int stage = 1;
    int wave = 1;
    int currency = 0;
    PlayerStats playerStats;
};

} // namespace controller