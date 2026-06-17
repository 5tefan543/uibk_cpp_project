#pragma once

#include "config/game_config.hpp"
#include "game/ecs/registry.hpp"
#include "game/location_table.hpp"

namespace game {

struct EnemyAI {
  private:
    void updateEnemyVelocityTowardsPlayer(Registry &registry, LocationTable &locationTable, const Position &playerPos,
                                          Entity enemy);
    void updateEnemyAnimationState(Registry &registry, Entity enemy, float dt);
    void applyAnimationMoveSpeedModifier(Registry &registry, const config::GameConfig &config, Entity enemyEntity);

  public:
    void update(Registry &registry, const config::GameConfig &config, LocationTable &locationTable, float dt);
};

} // namespace game