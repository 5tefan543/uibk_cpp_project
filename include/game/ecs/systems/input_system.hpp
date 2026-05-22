#pragma once

#include "controller/input/input_state.hpp"
#include "game/ecs/registry.hpp"

namespace game {

class InputSystem {
  private:
    void attack(Registry &registry, const PlayerStats &stats, Entity entity);
    void updateCooldown(float dt);
    float timeSinceLastAttack_ = 0.0f;

  public:
    void update(Registry &registry, const controller::InputState &input, float dt);
};

} // namespace game