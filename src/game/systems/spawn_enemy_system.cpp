#include "game/ecs/systems/spawn_enemy_system.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/boss_tag.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/velocity.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>

namespace game {

namespace {

constexpr float pi = 3.14159265358979323846f;
const std::string texturePathSuffix = "right_1.png";

} // namespace

SpawnEnemySystem::SpawnEnemySystem() : randomEngine_(std::random_device{}()) {}

void SpawnEnemySystem::update(Registry &registry, int wave, const controller::GameConfig &config)
{
    clearEnemies(registry);

    const SpawnContext context = createSpawnContext(registry);
    const controller::EnemySpawnConfig &spawnConfig = config.enemyConfig.spawn;

    const int enemyCount = generateEnemyCount(wave, config.maxEnemyCount, spawnConfig);
    for (int i = 0; i < enemyCount; ++i) {
        const controller::EnemyArchetypeConfig &archetype = chooseEnemyArchetype(config.enemyConfig, false);
        spawnEnemy(registry, wave, archetype, spawnConfig, context);
    }

    if (isBossWave(wave, config.wavesPerStage)) {
        const controller::EnemyArchetypeConfig &archetype = chooseEnemyArchetype(config.enemyConfig, true);
        spawnEnemy(registry, wave, archetype, spawnConfig, context);
    }
}

SpawnEnemySystem::SpawnContext SpawnEnemySystem::createSpawnContext(Registry &registry) const
{
    const auto mapEntities = registry.view<MapTag, view::Sprite>();
    const auto playerEntities = registry.view<PlayerTag, Position, PlayerStats, view::Sprite>();

    if (mapEntities.empty() || playerEntities.empty()) {
        throw std::runtime_error("No map or player entity found when trying to spawn enemy");
    }

    const Entity map = mapEntities.front();
    const Entity player = playerEntities.front();

    return SpawnContext{
        .mapSprite = registry.getComponent<view::Sprite>(map),
        .playerPosition = registry.getComponent<Position>(player),
        .playerSprite = registry.getComponent<view::Sprite>(player),
        .playerStats = registry.getComponent<PlayerStats>(player),
    };
}

void SpawnEnemySystem::clearEnemies(Registry &registry)
{
    for (Entity enemy : registry.view<EnemyTag>()) {
        registry.destroyEntity(enemy);
    }
}

int SpawnEnemySystem::generateEnemyCount(int wave, int maxEnemyCount, const controller::EnemySpawnConfig &spawnConfig)
{
    const float averageEnemyCount =
        spawnConfig.baseEnemyCount + static_cast<float>(wave) * spawnConfig.enemyCountGrowthPerWave;
    const float variation = std::max(1.0f, averageEnemyCount * spawnConfig.enemyCountVariationRatio);

    std::normal_distribution<float> enemyCountDistribution(averageEnemyCount, variation);
    const int sampledEnemyCount = static_cast<int>(std::round(enemyCountDistribution(randomEngine_)));

    return std::clamp(sampledEnemyCount, 1, maxEnemyCount);
}

bool SpawnEnemySystem::isBossWave(int wave, int wavesPerStage) const
{
    return wavesPerStage > 0 && wave % wavesPerStage == 0;
}

const controller::EnemyArchetypeConfig &
SpawnEnemySystem::chooseEnemyArchetype(const controller::EnemyConfig &enemyConfig, bool isBoss)
{
    std::vector<const controller::EnemyArchetypeConfig *> candidates;
    std::vector<double> weights;

    for (const controller::EnemyArchetypeConfig &archetype : enemyConfig.archetypes) {
        if (archetype.isBoss != isBoss) {
            continue;
        }

        candidates.push_back(&archetype);
        weights.push_back(std::max(0.0f, archetype.spawnWeight));
    }

    if (candidates.empty()) {
        throw std::runtime_error("No enemy archetype found for requested spawn type");
    }

    std::discrete_distribution<std::size_t> distribution(weights.begin(), weights.end());
    return *candidates.at(distribution(randomEngine_));
}

void SpawnEnemySystem::spawnEnemy(Registry &registry, int wave, const controller::EnemyArchetypeConfig &archetype,
                                  const controller::EnemySpawnConfig &spawnConfig, const SpawnContext &context)
{
    const std::string &baseTexturePath = archetype.baseTexturePath;

    view::Sprite enemySprite{
        .imagePath = baseTexturePath + texturePathSuffix,
    };

    Position spawnPosition = generateSpawnPosition(context, enemySprite, archetype.isBoss, spawnConfig);
    EnemyStats enemyStats = createEnemyStats(wave, archetype, spawnConfig, context);

    Entity enemy = registry.createEntity();
    registry.addComponent<EnemyTag>(enemy, {});
    registry.addComponent<Position>(enemy, spawnPosition);
    registry.addComponent<Velocity>(enemy, {});
    registry.addComponent<EnemyStats>(enemy, enemyStats);
    registry.addComponent<Animation>(enemy, {.baseTexturePath = baseTexturePath});
    registry.addComponent<view::Sprite>(enemy, enemySprite);

    if (archetype.isBoss) {
        registry.addComponent<BossTag>(enemy, {});
    }
}

Position SpawnEnemySystem::generateSpawnPosition(const SpawnContext &context, const view::Sprite &enemySprite,
                                                 bool isBoss, const controller::EnemySpawnConfig &spawnConfig)
{
    if (isBoss) {
        return generateBossSpawnPosition(context, enemySprite, spawnConfig);
    }

    return generateRandomSpawnPosition(context, enemySprite);
}

Position SpawnEnemySystem::generateBossSpawnPosition(const SpawnContext &context, const view::Sprite &enemySprite,
                                                     const controller::EnemySpawnConfig &spawnConfig)
{
    std::uniform_real_distribution<float> angleDistribution(0.0f, 2.0f * pi);
    std::uniform_real_distribution<float> distanceDistribution(spawnConfig.minBossSpawnDistanceFromPlayer,
                                                               spawnConfig.maxBossSpawnDistanceFromPlayer);

    const float angle = angleDistribution(randomEngine_);
    const float distance = distanceDistribution(randomEngine_);

    const float playerCenterX = context.playerPosition.x + context.playerSprite.width / 2.0f;
    const float playerCenterY = context.playerPosition.y + context.playerSprite.height / 2.0f;

    const float x = playerCenterX + std::cos(angle) * distance;
    const float y = playerCenterY + std::sin(angle) * distance;

    return {std::clamp(x, enemySprite.width, context.mapSprite.width - enemySprite.width),
            std::clamp(y, enemySprite.height, context.mapSprite.height - enemySprite.height)};
}

Position SpawnEnemySystem::generateRandomSpawnPosition(const SpawnContext &context, const view::Sprite &enemySprite)
{
    std::uniform_real_distribution<float> posXDistribution(enemySprite.width,
                                                           context.mapSprite.width - enemySprite.width);
    std::uniform_real_distribution<float> posYDistribution(enemySprite.height,
                                                           context.mapSprite.height - enemySprite.height);

    return {posXDistribution(randomEngine_), posYDistribution(randomEngine_)};
}

EnemyStats SpawnEnemySystem::createEnemyStats(int wave, const controller::EnemyArchetypeConfig &archetype,
                                              const controller::EnemySpawnConfig &spawnConfig,
                                              const SpawnContext &context)
{
    const float combatScaling = generateCombatScaling(wave, archetype, spawnConfig);

    EnemyStats stats;
    stats.maxHealth = archetype.stats.maxHealth * combatScaling;
    stats.health = stats.maxHealth;
    stats.attackPower = archetype.stats.attackPower * combatScaling;
    stats.attackSpeed = archetype.stats.attackSpeed;
    stats.defense = archetype.stats.defense * combatScaling;
    stats.moveSpeed = generateEnemyMoveSpeed(wave, archetype, spawnConfig, context);
    stats.scoreReward =
        std::max(1, static_cast<int>(std::round(static_cast<float>(archetype.scoreReward) * combatScaling)));

    return stats;
}

float SpawnEnemySystem::generateCombatScaling(int wave, const controller::EnemyArchetypeConfig &archetype,
                                              const controller::EnemySpawnConfig &spawnConfig)
{
    const float waveScaling =
        spawnConfig.baseEnemyScaling + static_cast<float>(wave - 1) * spawnConfig.enemyScalingGrowthPerWave;

    std::normal_distribution<float> variationDistribution(spawnConfig.enemyScalingVariationMean,
                                                          spawnConfig.enemyScalingVariationStddev);

    const float variation = std::clamp(variationDistribution(randomEngine_), spawnConfig.minEnemyScalingVariation,
                                       spawnConfig.maxEnemyScalingVariation);

    return waveScaling * variation * archetype.combatScaleMultiplier;
}

float SpawnEnemySystem::generateEnemyMoveSpeed(int wave, const controller::EnemyArchetypeConfig &archetype,
                                               const controller::EnemySpawnConfig &spawnConfig,
                                               const SpawnContext &context)
{
    const float wantedMoveSpeed =
        spawnConfig.baseEnemyMoveSpeed + static_cast<float>(wave - 1) * spawnConfig.enemyMoveSpeedGrowthPerWave;

    std::normal_distribution<float> variationDistribution(spawnConfig.enemyMoveSpeedVariationMean,
                                                          spawnConfig.enemyMoveSpeedVariationStddev);

    const float variation = std::clamp(variationDistribution(randomEngine_), spawnConfig.minEnemyMoveSpeedVariation,
                                       spawnConfig.maxEnemyMoveSpeedVariation);

    const float variedMoveSpeed = wantedMoveSpeed * variation;

    const float maxMoveSpeed = context.playerStats.moveSpeed * archetype.moveSpeedRatioOfPlayer;

    return std::min(variedMoveSpeed, maxMoveSpeed);
}

} // namespace game