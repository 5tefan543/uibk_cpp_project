#pragma once
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/stats.hpp"
#include "geometry/rectangle.hpp"
#include "geometry/vector.hpp"
#include "logging/log.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace config {

struct WindowConfig {
    int width;
    int height;
    std::string title;
};

struct AssetConfig {
    std::string meleeTexturePathPrefix;
    std::string rangedTexturePathPrefix;
    std::string enemyTexturePathPrefix;
    std::string mapTexturePathPrefix;
    std::string fontPath;
    std::string projectilePath;
    struct DroppableItemAssetConfig {
        std::string id;
        std::string spritePath;
    };

    std::vector<DroppableItemAssetConfig> droppableItems;
};

struct TextureConfig {
    std::string path;
    Vec2<float> position = {0, 0};
    Vec2<float> size = {0, 0};
};

struct HitBoxConfig {
    // TODO: store hitbox boundaries here
};

struct SpriteConfig {
    TextureConfig texture;
    HitBoxConfig hitBox;
};

struct DirectionalAnimationConfig {
    float frameDuration;
    std::unordered_map<game::AnimationDirection, std::vector<SpriteConfig>> directionToFrames;
};

struct AnimationConfig {
    std::unordered_map<game::AnimationState, DirectionalAnimationConfig> stateToDirection;
};

struct AnimationOverwriteConfig {
    std::string texturePathPrefix;
    float frameDuration = 0.16f;
    int totalFrames = 2;
    float moveSpeedMultiplier = 1.0f;
};

struct CombatStatsConfig {
    float maxHealth = 1.0f;
    float attackPower = 1.0f;
    float attackSpeed = 1.0f;
    float defense = 0.0f;
    float moveSpeed = 0.0f;
    float speedOfAttack = 0.0f;
    float attackRange = 0.0f;
};

struct ProjectileAttackConfig {
    float velocityScale = 10.0f;
    AnimationConfig animations;
};

struct MeleeArcAttackConfig {
    float reach = 0.0f;
    float hitBoxWidth = 64.0f;
    float hitBoxHeight = 64.0f;
    float activeTimePaddingSec = 0.1f;
};

struct BeamAttackConfig {
    float length = 0.0f;
    float width = 0.0f;
    float activeTimeSec = 0.0f;
};

struct AreaAttackConfig {
    float radius = 0.0f;
    float activeTimeSec = 0.0f;
};

struct AttackProfileConfig {
    game::DamageKind kind = game::DamageKind::Projectile;
    float amount = 1.0f;
    bool isMultiHit = false;
    float pushBackForce = 0.0f;
    float stunChance = 0.0f;
    AnimationOverwriteConfig animationOverwrite;
    ProjectileAttackConfig projectile;
    MeleeArcAttackConfig meleeArc;
    BeamAttackConfig beam;
    AreaAttackConfig area;
};

struct PlayerClassConfig {
    game::CharacterType characterType = game::CharacterType::Melee;
    bool hasDash = false;
    AnimationOverwriteConfig deathOverwrite;
    CombatStatsConfig stats;
    AttackProfileConfig attack;
    AnimationConfig animations;
    // SoundConfig
};

struct PlayerClassConfigs {
    PlayerClassConfig melee;
    PlayerClassConfig ranged;

    const PlayerClassConfig &getByType(game::CharacterType type) const
    {
        if (type == game::CharacterType::Melee) {
            return melee;
        }
        return ranged;
    }
};

struct EnemyClassConfig {
    bool isBoss = false;
    float spawnWeight = 1.0f;
    float combatScaleMultiplier = 1.0f;
    float moveSpeedRatioOfPlayer = 0.9f;
    std::string baseTexturePath;
    AnimationOverwriteConfig deathOverwrite;
    CombatStatsConfig stats;
    AttackProfileConfig attack;
    int scoreReward = 1;
    AnimationConfig animations;
};

struct EnemyClassConfigs {
    EnemyClassConfig blob;
    EnemyClassConfig boss;
};

struct EnemySpawnConfig {
    float baseEnemyCount = 2.0f;
    float enemyCountGrowthPerWave = 1.5f;
    float enemyCountVariationRatio = 0.25f;

    float minBossSpawnDistanceFromPlayer = 280.0f;
    float maxBossSpawnDistanceFromPlayer = 570.0f;

    float baseEnemyScaling = 1.0f;
    float enemyScalingGrowthPerWave = 0.15f;
    float enemyScalingVariationMean = 1.0f;
    float enemyScalingVariationStddev = 0.10f;
    float minEnemyScalingVariation = 0.8f;
    float maxEnemyScalingVariation = 1.2f;

    float baseEnemyMoveSpeed = 100.0f;
    float enemyMoveSpeedGrowthPerWave = 3.0f;
    float enemyMoveSpeedVariationMean = 1.0f;
    float enemyMoveSpeedVariationStddev = 0.08f;
    float minEnemyMoveSpeedVariation = 0.9f;
    float maxEnemyMoveSpeedVariation = 1.1f;
};

struct LogConfig {
    logger::LogLevel level;
    bool useColor;
};

struct MapConfig {
    Vec2<float> mapSize;
    std::vector<SpriteConfig> mapSprites;
};

struct GameConfig {
    int initialStage;
    int initialWave;
    int initialCurrency;
    int waveDurationSeconds;
    int wavesPerStage;
    int maxEnemyCount;
    WindowConfig windowConfig;
    LogConfig logConfig;
    MapConfig mapConfig;
    AssetConfig assetConfig;
    SpriteConfig fallbackSprite;
    PlayerClassConfigs playerClasses;
    EnemyClassConfigs enemyClasses;
    EnemySpawnConfig enemySpawnConfig;
    Vec2<unsigned> locTabNumBuckets;
};

} // namespace config