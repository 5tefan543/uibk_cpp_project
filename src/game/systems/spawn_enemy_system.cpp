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

// TODO: move to config

constexpr float baseEnemyCount = 2.0f;
constexpr float enemyCountGrowthPerWave = 1.5f;
constexpr float enemyCountVariationRatio = 0.25f;

constexpr float minBossSpawnDistanceFromPlayer = 280.0f;
constexpr float maxBossSpawnDistanceFromPlayer = 570.0f;

constexpr float baseEnemyScaling = 1.0f;
constexpr float enemyScalingGrowthPerWave = 0.15f;
constexpr float enemyScalingVariationMean = 1.0f;
constexpr float enemyScalingVariationStddev = 0.10f;
constexpr float minEnemyScalingVariation = 0.8f;
constexpr float maxEnemyScalingVariation = 1.2f;
constexpr float bossScalingMultiplier = 4.0f;

constexpr float baseEnemyHealth = 10.0f;
constexpr float baseEnemyAttackPower = 2.0f;
constexpr float baseEnemyAttackSpeed = 1.0f;
constexpr float baseEnemyDefense = 0.25f;

constexpr float baseEnemyMoveSpeed = 100.0f;
constexpr float enemyMoveSpeedGrowthPerWave = 3.0f;
constexpr float maxEnemySpeedRatioOfPlayer = 0.9f;
constexpr float maxBossMoveSpeedRatioOfPlayer = 0.5f;
constexpr float enemyMoveSpeedVariationMean = 1.0f;
constexpr float enemyMoveSpeedVariationStddev = 0.08f;
constexpr float minEnemyMoveSpeedVariation = 0.9f;
constexpr float maxEnemyMoveSpeedVariation = 1.1f;

const std::string enemyBaseTexturePath = "assets/characters/blob_enemy/blob_";
const std::string bossBaseTexturePath = "assets/characters/boss_1_";
const std::string texturePathSuffix = "right_1.png";

} // namespace

SpawnEnemySystem::SpawnEnemySystem() : randomEngine_(std::random_device{}()) {}

void SpawnEnemySystem::update(Registry &registry, int wave, const controller::GameConfig &config)
{
    clearEnemies(registry);

    auto mapEntities = registry.view<MapTag, view::Sprite>();
    auto playerEntities = registry.view<PlayerTag, Position, PlayerStats, view::Sprite>();

    if (mapEntities.empty() || playerEntities.empty()) {
        throw std::runtime_error("No map or player entity found when trying to spawn enemy");
    }

    // TODO: refactor ECS to use std::deque instead of std::vector
    // Since we get references to components we need to be careful to not invalidate them by creating new entities and
    // adding components Using a std::deque would solve this problem since it preserves references.

    // const view::Sprite &mapSprite = registry.getComponent<view::Sprite>(mapEntities.front());
    // const Position &playerPos = registry.getComponent<Position>(playerEntities.front());
    // const PlayerStats &playerStats = registry.getComponent<PlayerStats>(playerEntities.front());
    // const view::Sprite &playerSprite = registry.getComponent<view::Sprite>(playerEntities.front());

    // For now just copy the components to avoid reference invalidation issues -> REMOVE LATER
    const view::Sprite mapSprite = registry.getComponent<view::Sprite>(mapEntities.front());
    const Position playerPos = registry.getComponent<Position>(playerEntities.front());
    const PlayerStats playerStats = registry.getComponent<PlayerStats>(playerEntities.front());
    const view::Sprite playerSprite = registry.getComponent<view::Sprite>(playerEntities.front());

    const int enemyCount = generateEnemyCount(wave, config.maxEnemyCount);
    for (int i = 0; i < enemyCount; ++i) {
        spawnEnemy(registry, wave, false, mapSprite, playerPos, playerSprite, playerStats);
    }

    if (isBossWave(wave, config.wavesPerStage)) {
        spawnEnemy(registry, wave, true, mapSprite, playerPos, playerSprite, playerStats);
    }
}

void SpawnEnemySystem::clearEnemies(Registry &registry)
{
    for (Entity enemy : registry.view<EnemyTag>()) {
        registry.destroyEntity(enemy);
    }
}

int SpawnEnemySystem::generateEnemyCount(int wave, int maxEnemyCount)
{
    const float averageEnemyCount = baseEnemyCount + static_cast<float>(wave) * enemyCountGrowthPerWave;
    const float variation = std::max(1.0f, averageEnemyCount * enemyCountVariationRatio);

    std::normal_distribution<float> enemyCountDistribution(averageEnemyCount, variation);
    const int sampledEnemyCount = static_cast<int>(std::round(enemyCountDistribution(randomEngine_)));

    return std::clamp(sampledEnemyCount, 1, maxEnemyCount);
}

bool SpawnEnemySystem::isBossWave(int wave, int wavesPerStage) const
{
    return wavesPerStage > 0 && wave % wavesPerStage == 0;
}

void SpawnEnemySystem::spawnEnemy(Registry &registry, int wave, bool isBoss, const view::Sprite &mapSprite,
                                  const Position &playerPos, const view::Sprite &playerSprite,
                                  const PlayerStats &playerStats)
{
    const std::string baseTexturePath = getBaseTexturePath(isBoss);

    view::Sprite enemySprite{
        .imagePath = baseTexturePath + texturePathSuffix,
    };

    Position spawnPosition = generateSpawnPosition(mapSprite, playerPos, playerSprite, enemySprite, isBoss);
    EnemyStats enemyStats = createEnemyStats(wave, isBoss, playerStats.moveSpeed);

    Entity enemy = registry.createEntity();
    registry.addComponent<EnemyTag>(enemy, {});
    registry.addComponent<Position>(enemy, spawnPosition);
    registry.addComponent<Velocity>(enemy, {});
    registry.addComponent<EnemyStats>(enemy, enemyStats);
    registry.addComponent<Animation>(enemy, {.baseTexturePath = baseTexturePath});
    registry.addComponent<view::Sprite>(enemy, enemySprite);

    if (isBoss) {
        registry.addComponent<BossTag>(enemy, {});
    }
}

Position SpawnEnemySystem::generateSpawnPosition(const view::Sprite &mapSprite, const Position &playerPos,
                                                 const view::Sprite &playerSprite, const view::Sprite &enemySprite,
                                                 bool isBoss)
{
    if (isBoss) {
        return generateBossSpawnPosition(mapSprite, playerPos, playerSprite, enemySprite);
    }

    return generateRandomSpawnPosition(mapSprite, enemySprite);
}

Position SpawnEnemySystem::generateBossSpawnPosition(const view::Sprite &mapSprite, const Position &playerPos,
                                                     const view::Sprite &playerSprite, const view::Sprite &enemySprite)
{
    std::uniform_real_distribution<float> angleDistribution(0.0f, 2.0f * pi);
    std::uniform_real_distribution<float> distanceDistribution(minBossSpawnDistanceFromPlayer,
                                                               maxBossSpawnDistanceFromPlayer);

    const float angle = angleDistribution(randomEngine_);
    const float distance = distanceDistribution(randomEngine_);

    float playerCenterX = playerPos.x + playerSprite.width / 2.0f;
    float playerCenterY = playerPos.y + playerSprite.height / 2.0f;

    const float x = playerCenterX + std::cos(angle) * distance;
    const float y = playerCenterY + std::sin(angle) * distance;

    return {std::clamp(x, enemySprite.width, mapSprite.width - enemySprite.width),
            std::clamp(y, enemySprite.height, mapSprite.height - enemySprite.height)};
}

Position SpawnEnemySystem::generateRandomSpawnPosition(const view::Sprite &mapSprite, const view::Sprite &enemySprite)
{
    std::uniform_real_distribution<float> posXDistribution(enemySprite.width, mapSprite.width - enemySprite.width);
    std::uniform_real_distribution<float> posYDistribution(enemySprite.height, mapSprite.height - enemySprite.height);

    return {posXDistribution(randomEngine_), posYDistribution(randomEngine_)};
}

std::string SpawnEnemySystem::getBaseTexturePath(bool isBoss) const
{
    if (isBoss) {
        return bossBaseTexturePath;
    }

    return enemyBaseTexturePath;
}

EnemyStats SpawnEnemySystem::createEnemyStats(int wave, bool isBoss, float playerMoveSpeed)
{
    const float combatScaling = generateCombatScaling(wave, isBoss);

    EnemyStats stats;
    stats.maxHealth = baseEnemyHealth * combatScaling;
    stats.health = stats.maxHealth;
    stats.attackPower = baseEnemyAttackPower * combatScaling;
    stats.attackSpeed = baseEnemyAttackSpeed;
    stats.defense = baseEnemyDefense * combatScaling;
    stats.moveSpeed = generateEnemyMoveSpeed(wave, isBoss, playerMoveSpeed);
    stats.scoreReward = std::max(1, static_cast<int>(std::round(combatScaling)));

    return stats;
}

float SpawnEnemySystem::generateCombatScaling(int wave, bool isBoss)
{
    const float waveScaling = baseEnemyScaling + static_cast<float>(wave - 1) * enemyScalingGrowthPerWave;

    std::normal_distribution<float> variationDistribution(enemyScalingVariationMean, enemyScalingVariationStddev);

    const float variation =
        std::clamp(variationDistribution(randomEngine_), minEnemyScalingVariation, maxEnemyScalingVariation);

    float scaling = waveScaling * variation;

    if (isBoss) {
        scaling *= bossScalingMultiplier;
    }

    return scaling;
}

float SpawnEnemySystem::generateEnemyMoveSpeed(int wave, bool isBoss, float playerMoveSpeed)
{
    const float wantedMoveSpeed = baseEnemyMoveSpeed + static_cast<float>(wave - 1) * enemyMoveSpeedGrowthPerWave;

    std::normal_distribution<float> variationDistribution(enemyMoveSpeedVariationMean, enemyMoveSpeedVariationStddev);

    const float variation =
        std::clamp(variationDistribution(randomEngine_), minEnemyMoveSpeedVariation, maxEnemyMoveSpeedVariation);

    const float variedMoveSpeed = wantedMoveSpeed * variation;

    float maxMoveSpeed = playerMoveSpeed;

    if (isBoss) {
        maxMoveSpeed *= maxBossMoveSpeedRatioOfPlayer;
    } else {
        maxMoveSpeed *= maxEnemySpeedRatioOfPlayer;
    }

    return std::min(variedMoveSpeed, maxMoveSpeed);
}

} // namespace game