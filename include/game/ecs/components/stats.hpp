#pragma once

namespace game {

struct Stats {
    float maxHealth = 1.0f;
    float health = 1.0f;
    float attackPower = 1.0f;
    float attackSpeed = 1.0f;
    float defense = 0.0f;
    float moveSpeed = 0.0f;
};

struct PlayerStats : Stats {
    bool hasDash = false;
};

struct EnemyStats : Stats {
    int scoreReward = 1;
};

} // namespace game
