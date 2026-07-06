#pragma once

#include "config/game_config.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/registry.hpp"

namespace game {

struct BlobAttackSystem {
  private:
    void updateCoolDowns(Registry &registry, Entity enemyEntity, float dtSec);
    void triggerBlobAreaAttack(Registry &registry, const config::GameConfig &config, Entity blobEntity,
                               const Position &playerPosition);
    void spawnPendingAreaAttack(Registry &registry, const config::GameConfig &config, Entity blobEntity);

  public:
    void update(Registry &registry, const config::GameConfig &config, float dtSec);
};

} // namespace game