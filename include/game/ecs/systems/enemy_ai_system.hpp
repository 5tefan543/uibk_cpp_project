#pragma once

#include "config/game_config.hpp"
#include "game/ecs/registry.hpp"
#include "game/location_table.hpp"
#include <map>
namespace game {

struct EnemyAI {
  private:
    std::map<Entity, float> attackCoolDowns_;
    void blobAreaAttack(Registry &registry, const config::GameConfig &config, Entity blobEntity,
                        const Position &playerPosition);
    void updateEnemyVelocityTowardsPlayer(Registry &registry, LocationTable &locationTable, const Position &playerPos,
                                          Entity enemy);
    void updateEnemyAnimationState(Registry &registry, Entity enemy, float dt);
    void applyAnimationMoveSpeedModifier(Registry &registry, const config::GameConfig &config, Entity enemyEntity);
    void updateAttack(Registry &registry, const config::GameConfig &config, Entity enemyEntity,
                      const Position &playerPos);
    void updateCoolDowns(Registry &registry, Entity enemyEntity, float dt);

  public:
    void update(Registry &registry, const config::GameConfig &config, LocationTable &locationTable, float dt);
};

} // namespace game