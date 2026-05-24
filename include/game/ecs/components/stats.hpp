#pragma once

#include "game/ecs/components/damage.hpp"

namespace game {

struct Stats {
    float maxHealth = 1.0f;
    float health = 1.0f;
    float attackPower = 1.0f;
    float attackSpeed = 1.0f; // Attacks per seconds
    float defense = 0.0f;
    float moveSpeed = 0.0f;
    float speedOfAttack = 0.0f; // More travel speed of projectiles, or speed of swing animation etc..
    float attackRange = 0.0;
};

struct PlayerStats : Stats {
    bool hasDash = false;
    DamageKind dmgKind = DamageKind::Projectile;
    int enemiesPierced = 0;
    int score = 0;
    int currency = 0;
};

struct EnemyStats : Stats {
    int scoreReward = 1;
};

} // namespace game
