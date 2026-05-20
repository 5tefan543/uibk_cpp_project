#include "game/ecs/systems/spawn_enemy_system.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/boss_tag.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/velocity.hpp"
#include "view/sprite.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>

namespace game {

namespace {

constexpr float baseEnemyCount = 2.0f;
constexpr float enemyCountGrowthPerWave = 1.5f;
constexpr float enemyCountVariationRatio = 0.25f;

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

const std::string enemyBaseTexturePath = "assets/characters/enemy_1_";
const std::string bossBaseTexturePath = "assets/characters/boss_1_";
const std::string texturePathSuffix = "right_1.png";

} // namespace

SpawnEnemySystem::SpawnEnemySystem() : randomEngine_(std::random_device{}()) {}

void SpawnEnemySystem::update(Registry &registry, int wave, const controller::GameConfig &config)
{
    clearEnemies(registry);

    const int enemyCount = generateEnemyCount(wave, config.maxEnemyCount);
    for (int i = 0; i < enemyCount; ++i) {
        spawnEnemy(registry, wave, false);
    }

    if (isBossWave(wave, config.wavesPerStage)) {
        spawnEnemy(registry, wave, true);
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

void SpawnEnemySystem::spawnEnemy(Registry &registry, int wave, bool isBoss)
{
    auto mapEntities = registry.view<MapTag, view::Sprite>();

    if (mapEntities.empty()) {
        throw std::runtime_error("No map entity found when trying to spawn enemy");
    }

    view::Sprite enemySprite;
    view::Sprite &mapSprite = registry.getComponent<view::Sprite>(mapEntities.front());

    std::uniform_real_distribution<float> posXDistribution(enemySprite.width, mapSprite.width - enemySprite.width);
    std::uniform_real_distribution<float> posYDistribution(enemySprite.height, mapSprite.height - enemySprite.height);

    const std::string baseTexturePath = getEnemyBaseTexturePath(isBoss);

    Entity enemy = registry.createEntity();
    registry.addComponent<EnemyTag>(enemy, {});
    registry.addComponent<Position>(enemy, {posXDistribution(randomEngine_), posYDistribution(randomEngine_)});
    registry.addComponent<Velocity>(enemy, {});
    registry.addComponent<EnemyStats>(enemy, createEnemyStats(wave, isBoss));
    registry.addComponent<Animation>(enemy, {.baseTexturePath = baseTexturePath});
    registry.addComponent<view::Sprite>(enemy, {.imagePath = baseTexturePath + texturePathSuffix});

    if (isBoss) {
        registry.addComponent<BossTag>(enemy, {});
    }
}

std::string SpawnEnemySystem::getEnemyBaseTexturePath(bool isBoss) const
{
    if (isBoss) {
        return bossBaseTexturePath;
    }

    return enemyBaseTexturePath;
}

EnemyStats SpawnEnemySystem::createEnemyStats(int wave, bool isBoss)
{
    const float scaling = generateEnemyScaling(wave, isBoss);

    EnemyStats stats;
    stats.maxHealth = baseEnemyHealth * scaling;
    stats.health = stats.maxHealth;
    stats.attackPower = baseEnemyAttackPower * scaling;
    stats.attackSpeed = baseEnemyAttackSpeed * scaling;
    stats.defense = baseEnemyDefense * scaling;
    stats.moveSpeed = baseEnemyMoveSpeed * scaling;

    stats.scoreReward = static_cast<int>(std::round(scaling));

    return stats;
}

float SpawnEnemySystem::generateEnemyScaling(int wave, bool isBoss)
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

} // namespace game