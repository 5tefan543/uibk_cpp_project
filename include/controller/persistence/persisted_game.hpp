#pragma once

namespace controller {

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
    int score;
    int currency;
    PlayerStats playerStats;
};

} // namespace controller