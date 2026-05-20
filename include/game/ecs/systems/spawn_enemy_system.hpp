#pragma once

#include "controller/persistence/config_game.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/registry.hpp"
#include "view/sprite.hpp"
#include <random>

namespace game {

class SpawnEnemySystem {
  private:
    std::mt19937 randomEngine_;

    void clearEnemies(Registry &registry);
    int generateEnemyCount(int wave, int maxEnemyCount);
    bool isBossWave(int wave, int wavesPerStage) const;
    void spawnEnemy(Registry &registry, int wave, bool isBoss, const view::Sprite &mapSprite, const Position &playerPos,
                    const view::Sprite &playerSprite, const PlayerStats &playerStats);
    Position generateSpawnPosition(const view::Sprite &mapSprite, const Position &playerPos,
                                   const view::Sprite &playerSprite, const view::Sprite &enemySprite, bool isBoss);
    Position generateBossSpawnPosition(const view::Sprite &mapSprite, const Position &playerPos,
                                       const view::Sprite &playerSprite, const view::Sprite &enemySprite);
    Position generateRandomSpawnPosition(const view::Sprite &mapSprite, const view::Sprite &enemySprite);
    std::string getBaseTexturePath(bool isBoss) const;
    EnemyStats createEnemyStats(int wave, bool isBoss, float playerMoveSpeed);
    float generateCombatScaling(int wave, bool isBoss);
    float generateEnemyMoveSpeed(int wave, bool isBoss, float playerMoveSpeed);

  public:
    SpawnEnemySystem();
    void update(Registry &registry, int wave, const controller::GameConfig &config);
};

} // namespace game
