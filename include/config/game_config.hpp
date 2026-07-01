#pragma once
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/store/store_item.hpp"
#include "geometry/vector.hpp"
#include "logging/log.hpp"
#include "view/font.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace config {

struct WindowConfig {
    int width;
    int height;
    std::string title;
};

struct TextureConfig {
    std::string path;
    geometry::Vec2<float> position = {0, 0};
    geometry::Vec2<float> size = {128.0f, 128.0f};
};

struct HitBoxConfig {
    geometry::Vec2<float> offset = {0, 0};
    geometry::Vec2<float> size = {128.0f, 128.0f};
};

struct SpriteConfig {
    TextureConfig texture;
    HitBoxConfig hitBox;
};

struct AnimationStateConfig {
    float frameDuration = 0.16f;
    float moveSpeedMultiplier = 1.0f;
    std::unordered_map<game::AnimationDirection, std::vector<SpriteConfig>> directionToFrames;
};

struct AnimationConfig {
    std::unordered_map<game::AnimationState, AnimationStateConfig> stateToStateConfig;
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
    geometry::Vec2<float> hitBoxSize = {64.0f, 64.0f};
    float activeTimePaddingSec = 0.1f;
};

struct BeamAttackConfig {
    float length = 0.0f;
    float width = 0.0f;
    float activeTimeSec = 0.0f;
    int damageTicks = 0;
    AnimationConfig animations;
};

struct AreaAttackConfig {
    float radius = 0.0f;
    float activeTimeSec = 0.0f;
    int damageTicks = 0;
    float initialHit; // percentage based of damage amount the remaining damage will be divided over the damageTicks
    AnimationConfig animations;
};

struct AttackProfileConfig {
    game::DamageKind kind = game::DamageKind::Projectile;
    float amount = 1.0f;
    bool isMultiHit = false;
    float pushBackForce = 0.0f;
    float stunChance = 0.0f;
    ProjectileAttackConfig projectile;
    MeleeArcAttackConfig meleeArc;
    BeamAttackConfig beam;
    AreaAttackConfig area;
};

struct CharacterSoundConfig {
    std::string attack;
    std::string special;
    std::string hit;
    std::string dash;
};

struct PlayerClassConfig {
    game::CharacterType characterType = game::CharacterType::Melee;
    bool hasDash = false;
    CombatStatsConfig stats;
    AttackProfileConfig attack;
    AnimationConfig animations;
    CharacterSoundConfig sounds;
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
    game::EnemyType enemyType = game::EnemyType::Blob;
    float spawnWeight = 1.0f;
    float combatScaleMultiplier = 1.0f;
    float moveSpeedRatioOfPlayer = 0.9f;
    CombatStatsConfig stats;
    AttackProfileConfig attack;
    int scoreReward = 1;
    AnimationConfig animations;
    CharacterSoundConfig sounds;
};

struct EnemyClassConfigs {
    EnemyClassConfig blob;
    EnemyClassConfig boss;

    const EnemyClassConfig &getByType(game::EnemyType type) const
    {
        if (type == game::EnemyType::Blob) {
            return blob;
        }
        return boss;
    }
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
    geometry::Vec2<float> mapSize;
    std::vector<SpriteConfig> mapSprites;
};

struct LocationTableConfig {
    geometry::Vec2<unsigned> numBuckets;
};

struct FontConfig {
    std::unordered_map<view::FontType, std::string> fontToFilePath;
};

struct MenuSoundConfig {
    std::string gameMusic;
    std::string buttonHoverSound;
    std::string buttonClickSound;
    std::string gameOverSound;
    std::string waveOverSound;
    std::string storeMusic;
};

struct StoreItemTypeConfig {
    TextureConfig icon;
    game::PlayerStats statChanges = game::getDefaultPlayerStatChanges();
    int cost;
    float randomWeight;
};

struct StoreItemConfig {
    std::string name;
    std::string description;
    std::unordered_map<game::StoreItemType, StoreItemTypeConfig> typeToConfig;
};

struct StoreConfig {
    std::vector<StoreItemConfig> items;
    std::unordered_map<game::StoreItemType, float> typeToRandomWeight;
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
    LocationTableConfig locationTableConfig;
    FontConfig fontConfig;
    SpriteConfig fallbackSprite;
    PlayerClassConfigs playerClasses;
    EnemyClassConfigs enemyClasses;
    EnemySpawnConfig enemySpawnConfig;
    MenuSoundConfig menuSoundConfig;
    StoreConfig storeConfig;
};

} // namespace config