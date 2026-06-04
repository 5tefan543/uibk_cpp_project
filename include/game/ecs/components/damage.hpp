#pragma once
#include <variant>

namespace game {
enum class DamageKind {
    Projectile,
    MeleeArc,
    Beam,
    Area,
};

struct ProjectileDamage {
    float speed;            // units/sec
    float maxRange;         // projectile lifetime distance
    float distanceTraveled; // runtime state
    int targetsHit;
};

struct MeleeArcDamage {
    float reach;         // reach
    float activeTimeSec; // short hit window
    float elapsedSec;    // runtime state
};

struct BeamDamage {
    float length;        // reach
    float width;         // hitbox width
    float activeTimeSec; // short hit window
    float elapsedSec;    // runtime state
};

struct AreaDamage {
    float radius;        // reach
    float activeTimeSec; // short hit window
    float elapsedSec;    // runtime state
};

struct Damage {
    float amount; // shared damage value
    bool isMultiHit;
    float pushBackForce; // e.g. for knockback
    float stunChance;
    DamageKind kind;
    std::variant<ProjectileDamage, MeleeArcDamage, BeamDamage, AreaDamage> params;
};

} // namespace game