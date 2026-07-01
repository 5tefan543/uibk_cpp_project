#pragma once
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/registry.hpp"

namespace game {

struct DamageInformation {
    float actualDamageAmount = 0.0f;
    bool shouldBeRemoved = false;
};

class DamageSystem {
  private:
    DamageInformation updateMelee(Damage &damage, MeleeArcDamage &melee, float dtSec);
    DamageInformation updateProjectile(Registry &registry, Damage &damage, ProjectileDamage &projectile,
                                       Entity damageEntity, float dtSec);
    DamageInformation updateBeam(Damage &damage, BeamDamage &beam, DamageTag &tag, float dtSec);
    DamageInformation updateArea(Damage &damage, AreaDamage &area, DamageTag &tag, float dtSec);

  public:
    DamageSystem() = default;
    void update(Registry &registry, float dtSec);
};

} // namespace game