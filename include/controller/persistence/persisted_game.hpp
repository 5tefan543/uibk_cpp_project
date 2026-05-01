#pragma once

namespace controller {

// only temporary struct
// should be replaced by actual components and systems that manage them
struct PlayerStats {
    float maxHealth = 100.0f;
    float attackPower = 10.0f;
    float attackSpeed = 1.0f;
    float defense = 5.0f;
    float speed = 100.0f;
    bool hasDash = false;
};

struct PersistedGame {
    int stage = 1;
    int wave = 1;
    int currency = 0;
    PlayerStats playerStats;
};

} // namespace controller