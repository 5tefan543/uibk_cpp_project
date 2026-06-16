#pragma once

#include "config/game_config.hpp"
#include "controller/input/input_state.hpp"
#include "game/ecs/registry.hpp"

namespace game {

class InputSystem {
  private:
    float timeSinceLastAttack_ = 0.0f;
    float timeSinceLastSpecialMove_ = 0.0f;
    float timeSinceLastDash_ = 0.0f;

    void updateCooldown(float dt);
    void updatePlayerVelocity(Registry &registry, const Entity entity, const controller::InputState &input);
    void updatePlayerAnimationState(Registry &registry, const Entity entity, float dt);
    void handleAttack(Registry &registry, const config::GameConfig &config, Entity playerEntity,
                      const controller::InputState &input);
    void attackMelee(Registry &registry, const config::GameConfig &config, const Entity entity,
                     const controller::InputState &input, const config::AttackProfileConfig &attackProfile);
    void attackRanged(Registry &registry, const config::GameConfig &config, const Entity entity,
                      const controller::InputState &input, const config::AttackProfileConfig &attackProfile);
    void applyAnimationMoveSpeedModifier(Registry &registry, const config::GameConfig &config, Entity playerEntity);

  public:
    void update(Registry &registry, const config::GameConfig &config, const controller::InputState &input, float dt);
};

} // namespace game