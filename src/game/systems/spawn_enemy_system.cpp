#include "game/ecs/systems/spawn_enemy_system.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/boss_tag.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "view/sprite.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace game {

SpawnEnemySystem::SpawnEnemySystem() : randomEngine_(std::random_device{}()) {}

void SpawnEnemySystem::update(Registry &registry, int wave, int stage, const controller::GameConfig &config)
{
    // delete all existing enemies
    for (Entity enemy : registry.view<EnemyTag>()) {
        registry.destroyEntity(enemy);
    }

    const int enemyCount = calculateEnemyCount(wave, config.maxEnemyCount);
    for (int i = 0; i < enemyCount; ++i) {
        spawnEnemy(registry, wave, stage, false);
    }

    if (isBossWave(wave, config.wavesPerStage)) {
        spawnEnemy(registry, wave, stage, true);
    }
}

int SpawnEnemySystem::calculateEnemyCount(int wave, int maxEnemyCount)
{
    const float averageEnemyCount = 2.0f + static_cast<float>(wave) * 1.5f;
    const float variation = std::max(1.0f, averageEnemyCount * 0.25f);

    std::normal_distribution<float> enemyCountDistribution(averageEnemyCount, variation);
    const int sampledEnemyCount = static_cast<int>(std::round(enemyCountDistribution(randomEngine_)));

    return std::clamp(sampledEnemyCount, 1, maxEnemyCount);
}

bool SpawnEnemySystem::isBossWave(int wave, int wavesPerStage) const
{
    return wavesPerStage > 0 && wave % wavesPerStage == 0;
}

void SpawnEnemySystem::spawnEnemy(Registry &registry, int wave, int stage, bool isBoss)
{
    auto mapEntities = registry.view<MapTag, view::Sprite>();

    if (mapEntities.empty()) {
        throw std::runtime_error("No map entity found when trying to spawn enemy");
    }

    view::Sprite enemySprite;

    view::Sprite &mapSprite = registry.getComponent<view::Sprite>(mapEntities.front());
    std::uniform_real_distribution<float> posXDistribution(enemySprite.width, mapSprite.width - enemySprite.width);
    std::uniform_real_distribution<float> posYDistribution(enemySprite.height, mapSprite.height - enemySprite.height);

    Entity enemy = registry.createEntity();
    registry.addComponent<EnemyTag>(enemy, {});
    registry.addComponent<Position>(enemy, {posXDistribution(randomEngine_), posYDistribution(randomEngine_)});
    registry.addComponent<Velocity>(enemy, {0.0f, 0.0f});

    EnemyStats stats;
    const float scaling = 1.0f + static_cast<float>(wave - 1) * 0.15f + static_cast<float>(stage - 1) * 0.35f;
    stats.maxHealth = 10.0f * scaling;
    stats.health = stats.maxHealth;
    stats.attackPower = 2.0f * scaling;
    stats.attackSpeed = 1.0f;
    stats.defense = 0.5f * static_cast<float>(stage - 1);
    stats.moveSpeed = 100.0f + static_cast<float>(wave - 1) * 5.0f;
    stats.scoreReward = 5 + wave;

    Animation animation;

    if (isBoss) {
        registry.addComponent<BossTag>(enemy, {});
        stats.maxHealth *= 4.0f;
        stats.health = stats.maxHealth;
        stats.attackPower *= 2.0f;
        stats.defense += 2.0f;
        stats.scoreReward *= 5;
        animation.baseTexturePath = "assets/characters/boss_1_";
    } else {
        animation.baseTexturePath = "assets/characters/enemy_1_";
    }

    registry.addComponent<EnemyStats>(enemy, stats);
    registry.addComponent<Animation>(enemy, animation);
    registry.addComponent<view::Sprite>(enemy, {.imagePath = animation.baseTexturePath + "right_1.png"});
}

} // namespace game
