#pragma once

#include "controller/persistence/game_config.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/registry.hpp"
#include "view/sprite.hpp"
#include <random>

namespace game {

class SpawnEnemySystem {
  private:
    std::mt19937 randomEngine_;

    struct SpawnContext {
        view::Sprite mapSprite;
        Position playerPosition;
        view::Sprite playerSprite;
        PlayerStats playerStats;
        int spawnID;
    };

    SpawnContext createSpawnContext(Registry &registry) const;
    void clearEnemies(Registry &registry);
    int generateEnemyCount(int wave, int maxEnemyCount);
    bool isBossWave(int wave, int wavesPerStage) const;
    void spawnEnemy(Registry &registry, int wave, bool isBoss, const SpawnContext &context);
    Position generateSpawnPosition(const SpawnContext &context, const view::Sprite &enemySprite, bool isBoss);
    Position generateBossSpawnPosition(const SpawnContext &context, const view::Sprite &enemySprite);
    Position generateRandomSpawnPosition(const SpawnContext &context, const view::Sprite &enemySprite);
    std::string getBaseTexturePath(bool isBoss) const;
    EnemyStats createEnemyStats(int wave, bool isBoss, const SpawnContext &context);
    float generateCombatScaling(int wave, bool isBoss);
    float generateEnemyMoveSpeed(int wave, bool isBoss, const SpawnContext &context);

  public:
    SpawnEnemySystem();
    void update(Registry &registry, int wave, const controller::GameConfig &config);
};

} // namespace game
