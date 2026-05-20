#pragma once

#include "controller/persistence/config_game.hpp"
#include "game/ecs/registry.hpp"
#include <random>

namespace game {

class SpawnEnemySystem {
  private:
    std::mt19937 randomEngine_;

    int calculateEnemyCount(int wave, int maxEnemyCount);
    bool isBossWave(int wave, int wavesPerStage) const;
    void spawnEnemy(Registry &registry, int wave, int stage, bool isBoss);

  public:
    SpawnEnemySystem();
    void update(Registry &registry, int wave, int stage, const controller::GameConfig &config);
};

} // namespace game
