#pragma once

#include "controller/persistence/config_game.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/registry.hpp"
#include <random>

namespace game {

class SpawnEnemySystem {
  private:
    std::mt19937 randomEngine_;

    void clearEnemies(Registry &registry);
    int generateEnemyCount(int wave, int maxEnemyCount);
    bool isBossWave(int wave, int wavesPerStage) const;
    void spawnEnemy(Registry &registry, int wave, bool isBoss);
    std::string getEnemyBaseTexturePath(bool isBoss) const;
    EnemyStats createEnemyStats(int wave, bool isBoss);
    float generateEnemyScaling(int wave, bool isBoss);

  public:
    SpawnEnemySystem();
    void update(Registry &registry, int wave, const controller::GameConfig &config);
};

} // namespace game
