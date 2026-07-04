#pragma once

#include "config/game_config.hpp"
#include "controller/input/input_state.hpp"
#include "game/ecs/registry.hpp"

namespace game {

class InputSystem {
  private:
    void updateCooldown(Registry &registry, Entity entity, float dtSec);
    void updatePlayerVelocity(Registry &registry, const Entity entity, const controller::InputState &input);
    void updatePlayerAnimationState(Registry &registry, const Entity entity, float dtSec);
    void handleAttack(Registry &registry, const config::GameConfig &config, Entity playerEntity,
                      const controller::InputState &input);
    void attackMelee(Registry &registry, const config::GameConfig &config, const Entity entity,
                     const controller::InputState &input, const config::AttackProfileConfig &attackProfile);
    void attackRanged(Registry &registry, const config::GameConfig &config, const Entity entity,
                      const controller::InputState &input, const config::AttackProfileConfig &attackProfile,
                      bool specialAttack);
    void applyAnimationMoveSpeedModifier(Registry &registry, const config::GameConfig &config, Entity playerEntity);

  public:
    void update(Registry &registry, const config::GameConfig &config, const controller::InputState &input, float dtSec);
};

} // namespace game