#pragma once

#include "config/game_config.hpp"
#include "controller/input/input_state.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/registry.hpp"

namespace game {

class InputSystem {
  private:
    const config::AttackProfileConfig &getAttackProfile(const config::GameConfig &config,
                                                        CharacterType characterType) const;
    void attackRanged(Registry &registry, const PlayerStats &stats, const Entity entity,
                      const controller::InputState &input, const config::AttackProfileConfig &attackProfile,
                      const config::GameConfig &config);
    void attackMelee(Registry &registry, const PlayerStats &stats, const Entity entity,
                     const controller::InputState &input, const config::AttackProfileConfig &attackProfile);
    void updateCooldown(float dt);
    float timeSinceLastAttack_ = 0.0f;
    float timeSinceLastSpecialMove_ = 0.0f;
    float timeSinceLastDash_ = 0.0f;

  public:
    void update(Registry &registry, const config::GameConfig &config, const controller::InputState &input, float dt);
};

} // namespace game