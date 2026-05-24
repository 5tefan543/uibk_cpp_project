#pragma once
#include "game/ecs/registry.hpp"

namespace game {

class DamageSystem {
  private:
    void removeIfDead(Registry &registry, Entity entity);

  public:
    DamageSystem() = default;
    void update(Registry &registry, float dt);
};

} // namespace game