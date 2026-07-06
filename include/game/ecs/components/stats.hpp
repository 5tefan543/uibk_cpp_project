#pragma once

#include "game/ecs/components/damage.hpp"

namespace game {

enum class CharacterType { Melee, Ranged };
enum class EnemyType { Blob, Boss };

inline const char *toString(game::CharacterType type)
{
    switch (type) {
    case game::CharacterType::Melee:
        return "Melee";
    case game::CharacterType::Ranged:
        return "Ranged";
    default:
        return "Unknown";
    }
}

inline const char *toString(game::EnemyType type)
{
    switch (type) {
    case game::EnemyType::Blob:
        return "Blob";
    case game::EnemyType::Boss:
        return "Boss";
    default:
        return "Unknown";
    }
}

struct Stats {
    float maxHealth = 1.0f;
    float health = 1.0f;
    float attackPower = 1.0f;
    float attackSpeed = 1.0f;        // Attacks per seconds
    float specialAttackSpeed = 1.0f; // Special attacks per seconds
    float defense = 0.0f;
    float moveSpeed = 0.0f;
    float speedOfAttack = 0.0f; // More travel speed of projectiles, or speed of swing animation etc..
    float attackRange = 0.0;
    float healthRegen = 0.0f; // Health per second, default 0 (no regen)
};

struct PlayerStats : Stats {
    CharacterType characterType = CharacterType::Melee;
    bool hasDash = false;
    int enemiesPierced = 0;
    int score = 0;
    int currency = 0;
};

struct EnemyStats : Stats {
    EnemyType enemyType = EnemyType::Blob;
    int scoreReward = 1;
};

inline game::PlayerStats getDefaultPlayerStatChanges()
{
    game::PlayerStats stats;

    stats.maxHealth = 0.0f;
    stats.health = 0.0f;
    stats.attackPower = 0.0f;
    stats.attackSpeed = 0.0f;
    stats.specialAttackSpeed = 0.0f;
    stats.defense = 0.0f;
    stats.moveSpeed = 0.0f;
    stats.speedOfAttack = 0.0f;
    stats.attackRange = 0.0f;
    stats.healthRegen = 0.0f;

    stats.hasDash = false;
    stats.enemiesPierced = 0;
    stats.score = 0;
    stats.currency = 0;

    return stats;
}

} // namespace game
