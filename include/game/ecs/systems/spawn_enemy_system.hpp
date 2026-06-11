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

    struct SpawnContext {
        view::Sprite mapSprite;
        Position playerPosition;
        view::Sprite playerSprite;
        PlayerStats playerStats;
        int spawnID;
    };

    SpawnContext createSpawnContext(Registry &registry) const;
    void clearEnemies(Registry &registry);
    int generateEnemyCount(int wave, int maxEnemyCount, const controller::EnemySpawnConfig &spawnConfig);
    bool isBossWave(int wave, int wavesPerStage) const;
    const controller::EnemyArchetypeConfig &chooseEnemyArchetype(const controller::EnemyConfig &enemyConfig,
                                                                 bool isBoss);
    void spawnEnemy(Registry &registry, int wave, const controller::EnemyArchetypeConfig &archetype,
                    const controller::EnemySpawnConfig &spawnConfig, const SpawnContext &context);
    Position generateSpawnPosition(const SpawnContext &context, const view::Sprite &enemySprite, bool isBoss,
                                   const controller::EnemySpawnConfig &spawnConfig);
    Position generateBossSpawnPosition(const SpawnContext &context, const view::Sprite &enemySprite,
                                       const controller::EnemySpawnConfig &spawnConfig);
    Position generateRandomSpawnPosition(const SpawnContext &context, const view::Sprite &enemySprite);
    EnemyStats createEnemyStats(int wave, const controller::EnemyArchetypeConfig &archetype,
                                const controller::EnemySpawnConfig &spawnConfig, const SpawnContext &context);
    float generateCombatScaling(int wave, const controller::EnemyArchetypeConfig &archetype,
                                const controller::EnemySpawnConfig &spawnConfig);
    float generateEnemyMoveSpeed(int wave, const controller::EnemyArchetypeConfig &archetype,
                                 const controller::EnemySpawnConfig &spawnConfig, const SpawnContext &context);

  public:
    SpawnEnemySystem();
    void update(Registry &registry, int wave, const controller::GameConfig &config);
};

} // namespace game
