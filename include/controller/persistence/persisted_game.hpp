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
    int wave = 0;
    int score = 0;
    int currency = 0;
    PlayerStats playerStats;
};

} // namespace controller