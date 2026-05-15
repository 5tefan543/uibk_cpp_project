#pragma once

namespace controller {

struct PlayerStats {
    float posX = 0.0f;
    float posY = 0.0f;
    float maxHealth = 0.0f;
    float attackPower = 0.0f;
    float attackSpeed = 0.0f;
    float defense = 0.0f;
    float speed = 0.0f;
    bool hasDash = false;
};

struct PersistedGame {
    int stage;
    int wave;
    int score;
    int currency;
    PlayerStats playerStats;
};

} // namespace controller