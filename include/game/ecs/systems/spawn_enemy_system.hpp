#pragma once

#include "config/game_config.hpp"
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
    int generateEnemyCount(int wave, int maxEnemyCount, const config::EnemySpawnConfig &spawnConfig);
    bool isBossWave(int wave, int wavesPerStage) const;
    void spawnEnemy(Registry &registry, int wave, const config::EnemyClassConfig &archetype,
                    const config::EnemySpawnConfig &spawnConfig, const SpawnContext &context);
    Position generateSpawnPosition(const SpawnContext &context, const view::Sprite &enemySprite, bool isBoss,
                                   const config::EnemySpawnConfig &spawnConfig);
    Position generateBossSpawnPosition(const SpawnContext &context, const view::Sprite &enemySprite,
                                       const config::EnemySpawnConfig &spawnConfig);
    Position generateRandomSpawnPosition(const SpawnContext &context, const view::Sprite &enemySprite);
    EnemyStats createEnemyStats(int wave, const config::EnemyClassConfig &archetype,
                                const config::EnemySpawnConfig &spawnConfig, const SpawnContext &context);
    float generateCombatScaling(int wave, const config::EnemyClassConfig &archetype,
                                const config::EnemySpawnConfig &spawnConfig);
    float generateEnemyMoveSpeed(int wave, const config::EnemyClassConfig &archetype,
                                 const config::EnemySpawnConfig &spawnConfig, const SpawnContext &context);

  public:
    SpawnEnemySystem();
    void update(Registry &registry, int wave, const config::GameConfig &config);
};

} // namespace game
