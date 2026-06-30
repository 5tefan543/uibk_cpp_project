#include "game/ecs/systems/spawn_enemy_system.hpp"
#include "config/animation_config_helper.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/health_bar_state.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/map_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/velocity.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>

namespace game {

constexpr float pi = 3.14159265358979323846f;

SpawnEnemySystem::SpawnEnemySystem() : randomEngine_(std::random_device{}()) {}

void SpawnEnemySystem::update(Registry &registry, int wave, const config::GameConfig &config)
{
    clearEnemies(registry);

    const SpawnContext context = createSpawnContext(registry);
    const config::EnemySpawnConfig &spawnConfig = config.enemySpawnConfig;

    const int enemyCount = generateEnemyCount(wave, config.maxEnemyCount, spawnConfig);
    for (int i = 0; i < enemyCount; ++i) {
        spawnEnemy(registry, wave, config, EnemyType::Blob, spawnConfig, context);
    }

    if (isBossWave(wave, config.wavesPerStage)) {
        spawnEnemy(registry, wave, config, EnemyType::Boss, spawnConfig, context);
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

    return SpawnContext{.mapSprite = registry.getComponent<view::Sprite>(map),
                        .playerPosition = registry.getComponent<Position>(player),
                        .playerSprite = registry.getComponent<view::Sprite>(player),
                        .playerStats = registry.getComponent<PlayerStats>(player),
                        .spawnID = 0};
}

void SpawnEnemySystem::clearEnemies(Registry &registry)
{
    for (Entity enemy : registry.view<EnemyTag>()) {
        registry.destroyEntity(enemy);
    }
}

int SpawnEnemySystem::generateEnemyCount(int wave, int maxEnemyCount, const config::EnemySpawnConfig &spawnConfig)
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

void SpawnEnemySystem::spawnEnemy(Registry &registry, int wave, const config::GameConfig &config, EnemyType enemyType,
                                  const config::EnemySpawnConfig &spawnConfig, const SpawnContext &context)
{
    Animation enemyAnimation{
        .state = AnimationState::Idle,
        .direction = AnimationDirection::Right,
    };

    const config::AnimationFrame animationFrame = config::AnimationConfigHelper::getEnemyAnimationFrame(
        config, enemyType, enemyAnimation.state, enemyAnimation.direction, enemyAnimation.currentFrame);

    view::Sprite enemySprite{
        .rect = {{}, animationFrame.spriteConfig.texture.size},
        .imagePath = animationFrame.spriteConfig.texture.path,
    };

    Position spawnPosition = generateSpawnPosition(context, enemySprite, enemyType, spawnConfig);
    const config::EnemyClassConfig &enemyClass = config.enemyClasses.getByType(enemyType);
    EnemyStats enemyStats = createEnemyStats(wave, enemyClass, spawnConfig, context);

    HitBox hitBox{animationFrame.spriteConfig.hitBox.offset, animationFrame.spriteConfig.hitBox.size};

    Entity enemy = registry.createEntity();
    registry.addComponent<EnemyTag>(enemy, {});
    registry.addComponent<Position>(enemy, spawnPosition);
    registry.addComponent<Velocity>(enemy, {});
    registry.addComponent<EnemyStats>(enemy, enemyStats);
    registry.addComponent<Animation>(enemy, enemyAnimation);
    registry.addComponent<view::Sprite>(enemy, enemySprite);
    registry.addComponent<HitBox>(enemy, hitBox);
    registry.addComponent<HealthBarState>(enemy, {});
}

Position SpawnEnemySystem::generateSpawnPosition(const SpawnContext &context, const view::Sprite &enemySprite,
                                                 EnemyType enemyType, const config::EnemySpawnConfig &spawnConfig)
{
    if (enemyType == EnemyType::Boss) {
        return generateBossSpawnPosition(context, enemySprite, spawnConfig);
    }

    return generateRandomSpawnPosition(context, enemySprite);
}

Position SpawnEnemySystem::generateBossSpawnPosition(const SpawnContext &context, const view::Sprite &enemySprite,
                                                     const config::EnemySpawnConfig &spawnConfig)
{
    std::uniform_real_distribution<float> angleDistribution(0.0f, 2.0f * pi);
    std::uniform_real_distribution<float> distanceDistribution(spawnConfig.minBossSpawnDistanceFromPlayer,
                                                               spawnConfig.maxBossSpawnDistanceFromPlayer);

    const float angle = angleDistribution(randomEngine_);
    const float distance = distanceDistribution(randomEngine_);

    const geometry::Vec2<float> playerCenter = context.playerPosition.p + context.playerSprite.rect.size / 2.0f;

    const geometry::Vec2<float> pos = {.x = playerCenter.x + std::cos(angle) * distance,
                                       .y = playerCenter.y + std::sin(angle) * distance

    };

    return Position{pos.clamp(enemySprite.rect.size, context.mapSprite.rect.size - enemySprite.rect.size)};
}

Position SpawnEnemySystem::generateRandomSpawnPosition(const SpawnContext &context, const view::Sprite &enemySprite)
{
    std::uniform_real_distribution<float> posXDistribution(enemySprite.rect.size.x,
                                                           context.mapSprite.rect.size.x - enemySprite.rect.size.x);
    std::uniform_real_distribution<float> posYDistribution(enemySprite.rect.size.y,
                                                           context.mapSprite.rect.size.y - enemySprite.rect.size.y);

    return {{posXDistribution(randomEngine_), posYDistribution(randomEngine_)}};
}

EnemyStats SpawnEnemySystem::createEnemyStats(int wave, const config::EnemyClassConfig &classConfig,
                                              const config::EnemySpawnConfig &spawnConfig, const SpawnContext &context)
{
    const float combatScaling = generateCombatScaling(wave, classConfig, spawnConfig);

    EnemyStats stats;
    stats.maxHealth = classConfig.stats.maxHealth * combatScaling;
    stats.health = stats.maxHealth;
    stats.attackPower = classConfig.stats.attackPower * combatScaling;
    stats.attackSpeed = classConfig.stats.attackSpeed;
    stats.defense = classConfig.stats.defense * combatScaling;
    stats.moveSpeed = generateEnemyMoveSpeed(wave, classConfig, spawnConfig, context);
    stats.speedOfAttack = classConfig.stats.speedOfAttack;
    stats.attackRange = classConfig.stats.attackRange;
    stats.enemyType = classConfig.enemyType;
    stats.scoreReward =
        std::max(1, static_cast<int>(std::round(static_cast<float>(classConfig.scoreReward) * combatScaling)));

    return stats;
}

float SpawnEnemySystem::generateCombatScaling(int wave, const config::EnemyClassConfig &classConfig,
                                              const config::EnemySpawnConfig &spawnConfig)
{
    const float waveScaling =
        spawnConfig.baseEnemyScaling + static_cast<float>(wave - 1) * spawnConfig.enemyScalingGrowthPerWave;

    std::normal_distribution<float> variationDistribution(spawnConfig.enemyScalingVariationMean,
                                                          spawnConfig.enemyScalingVariationStddev);

    const float variation = std::clamp(variationDistribution(randomEngine_), spawnConfig.minEnemyScalingVariation,
                                       spawnConfig.maxEnemyScalingVariation);

    return waveScaling * variation * classConfig.combatScaleMultiplier;
}

float SpawnEnemySystem::generateEnemyMoveSpeed(int wave, const config::EnemyClassConfig &classConfig,
                                               const config::EnemySpawnConfig &spawnConfig, const SpawnContext &context)
{
    const float wantedMoveSpeed =
        spawnConfig.baseEnemyMoveSpeed + static_cast<float>(wave - 1) * spawnConfig.enemyMoveSpeedGrowthPerWave;

    std::normal_distribution<float> variationDistribution(spawnConfig.enemyMoveSpeedVariationMean,
                                                          spawnConfig.enemyMoveSpeedVariationStddev);

    const float variation = std::clamp(variationDistribution(randomEngine_), spawnConfig.minEnemyMoveSpeedVariation,
                                       spawnConfig.maxEnemyMoveSpeedVariation);

    const float variedMoveSpeed = wantedMoveSpeed * variation;

    const float maxMoveSpeed = context.playerStats.moveSpeed * classConfig.moveSpeedRatioOfPlayer;

    return std::min(variedMoveSpeed, maxMoveSpeed);
}

} // namespace game