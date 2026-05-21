#pragma once
#include <variant>

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
    float pushbackForce;    // e.g. for knockback
    int targetsHit;         // runtime state, e.g. for piercing projectiles
};

struct MeleeArcDamage {
    float arcAngleDeg;   // e.g. 90
    float arcRadius;     // reach
    float activeTimeSec; // short hit window
    float elapsedSec;    // runtime state
};

struct Damage {
    float amount;  // shared damage value
    bool isActive; // shared enable flag
    DamageKind kind;
    std::variant<ProjectileDamage, MeleeArcDamage> params;
};

// Ignore the following structs for now,

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