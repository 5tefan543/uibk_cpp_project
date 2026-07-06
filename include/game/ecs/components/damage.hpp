#pragma once
#include <variant>

namespace game {
enum class DamageKind {
    Projectile,
    Unicorn,
    MeleeArc,
    Beam,
    Area,
};

struct ProjectileDamage {
    float speed;            // units/sec
    float maxRange;         // projectile lifetime distance
    float distanceTraveled; // runtime state
    int maxTargets;
};

struct UnicornDamage {
    float speed; // units/sec
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
    int damageTicks;
    float elapsedSecSinceLastTick;
};

// e.g.
// activeTimeSec = 3
// damageTicks = 10
// initialhit = 0.4
// Damage::amount = 10
// then at the start of the attack 4 dmg is done immediately at the end of animation and spawning of the object
// then each 0.3 seconds there will be a tick of 0.6 dmg
struct AreaDamage {
    float radius;        // reach
    float activeTimeSec; // short hit window
    float elapsedSec;    // runtime state
    float telegraphTimeSec = 0.1f;
    float initialHit; // percentage based of damage amount the remaining damage will be divided over the damageTicks
    int damageTicks;
    float elapsedSecSinceLastTick;
};

struct Damage {
    float amount;        // shared damage value
    float pushBackForce; // e.g. for knockback
    float stunChance;
    DamageKind kind;
    enum class Mode { Flat, Percent, ScaledByAttack };
    Mode mode = Mode::Flat; // default: flat amount subtraction
    std::variant<ProjectileDamage, UnicornDamage, MeleeArcDamage, BeamDamage, AreaDamage> params;
};

} // namespace game