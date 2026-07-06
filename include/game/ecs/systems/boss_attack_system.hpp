#pragma once

#include "config/game_config.hpp"
#include "game/ecs/registry.hpp"
#include <random>

namespace game {

class BossAttackSystem {
  private:
    std::mt19937 randomEngine_;

    void updateCoolDown(Registry &registry, Entity enemyEntity, float dtSec);
    void ensurePhaseComponent(Registry &registry, Entity bossEntity);
    void updatePhase(Registry &registry, Entity bossEntity);
    void spawnRadialProjectileBurst(Registry &registry, const config::GameConfig &config, Entity bossEntity);
    void spawnPhaseTwoLightning(Registry &registry, const config::GameConfig &config, Entity bossEntity);

  public:
    BossAttackSystem();
    void update(Registry &registry, const config::GameConfig &config, float dtSec);
};

} // namespace game