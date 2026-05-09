#pragma once

namespace controller {

// only temporary struct
// should be replaced by actual components and systems that manage them
struct PlayerStats {
    float maxHealth;
    float attackPower;
    float attackSpeed;
    float defense;
    float speed;
    bool hasDash;
};

struct PersistedGame {
    int stage;
    int wave;
    int currency;
    PlayerStats playerStats;
};

} // namespace controller