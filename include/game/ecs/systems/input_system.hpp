#pragma once

#include "controller/input/input_state.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/registry.hpp"

namespace game {

class InputSystem {
  private:
    void attackRanged(Registry &registry, const PlayerStats &stats, const Entity entity,
                      const controller::InputState &input);
    void attackMelee(Registry &registry, const PlayerStats &stats, const Entity entity,
                     const controller::InputState &input);
    void updateCooldown(float dt);
    float timeSinceLastAttack_ = 0.0f;

  public:
    void update(Registry &registry, const controller::InputState &input, float dt);
};

} // namespace game