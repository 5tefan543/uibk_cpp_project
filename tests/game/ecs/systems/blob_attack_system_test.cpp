#include "config/game_config.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/damage.hpp"
#include "game/ecs/components/damage_tag.hpp"
#include "game/ecs/components/enemy_attack_tag.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/hitbox.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/blob_attack_system.hpp"
#include "game/ecs/systems/enemy_ai_system.hpp"
#include "game/ecs/systems/player_distance_system.hpp"
#include "game/location_table.hpp"
#include "shared/test_fixture.hpp"
#include "view/sprite.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <variant>

namespace {

config::SpriteConfig makeSpriteConfig(const std::string &path)
{
    config::SpriteConfig spriteConfig{};
    spriteConfig.texture.path = path;
    spriteConfig.texture.size = {32.0f, 32.0f};
    spriteConfig.hitBox.offset = {0.0f, 0.0f};
    spriteConfig.hitBox.size = {32.0f, 32.0f};
    return spriteConfig;
}

void addEnemyAnimationState(config::AnimationConfig &animations, game::AnimationState state,
                            game::AnimationDirection direction, float moveSpeedMultiplier = 1.0f)
{
    auto &stateConfig = animations.stateToStateConfig[state];
    stateConfig.frameDuration = 0.2f;
    stateConfig.moveSpeedMultiplier = moveSpeedMultiplier;
    stateConfig.directionToFrames[direction] = {
        makeSpriteConfig("enemy_frame_1.png"),
        makeSpriteConfig("enemy_frame_2.png"),
    };
}

config::GameConfig makeBlobAttackTestConfig()
{
    config::GameConfig config{};

    config.mapConfig.mapSize = {1000.0f, 1000.0f};
    config.locationTableConfig.numBuckets = {10, 10};

    config.enemyClasses.blob.enemyType = game::EnemyType::Blob;
    config.enemyClasses.blob.stats.attackSpeed = 1.0f;

    addEnemyAnimationState(config.enemyClasses.blob.animations, game::AnimationState::Idle,
                           game::AnimationDirection::Right);
    addEnemyAnimationState(config.enemyClasses.blob.animations, game::AnimationState::Idle,
                           game::AnimationDirection::Left);
    addEnemyAnimationState(config.enemyClasses.blob.animations, game::AnimationState::Walk,
                           game::AnimationDirection::Right);
    addEnemyAnimationState(config.enemyClasses.blob.animations, game::AnimationState::Walk,
                           game::AnimationDirection::Left);
    addEnemyAnimationState(config.enemyClasses.blob.animations, game::AnimationState::Attack,
                           game::AnimationDirection::Left);
    addEnemyAnimationState(config.enemyClasses.blob.animations, game::AnimationState::Attack,
                           game::AnimationDirection::Right);

    return config;
}

game::LocationTable makeLocationTable(const config::GameConfig &config)
{
    return game::LocationTable{
        config.locationTableConfig.numBuckets,
        config.mapConfig.mapSize,
    };
}

game::Entity addPlayer(game::Registry &registry, float x, float y)
{
    const game::Entity player = registry.createEntity();
    registry.addComponent<game::PlayerTag>(player, {});
    registry.addComponent<game::Position>(player, {{x, y}});
    return player;
}

game::Entity addBlob(game::Registry &registry, float x, float y, float moveSpeed = 0.0f)
{
    const game::Entity enemy = registry.createEntity();

    game::EnemyStats stats{};
    stats.enemyType = game::EnemyType::Blob;
    stats.moveSpeed = moveSpeed;

    game::Animation animation{};
    animation.state = game::AnimationState::Idle;
    animation.direction = game::AnimationDirection::Right;

    registry.addComponent<game::EnemyTag>(enemy, {});
    registry.addComponent<game::Position>(enemy, {{x, y}});
    registry.addComponent<game::Velocity>(enemy, {});
    registry.addComponent<game::EnemyStats>(enemy, stats);
    registry.addComponent<game::Animation>(enemy, animation);

    return enemy;
}

void runFrame(game::Registry &registry, const config::GameConfig &config, game::LocationTable &locationTable,
              game::EnemyAI &enemyAI, game::PlayerDistanceSystem &distanceSystem, game::BlobAttackSystem &blobSystem,
              float dtSec)
{
    locationTable.update(registry);
    enemyAI.update(registry, config, locationTable, dtSec);
    distanceSystem.update(registry);
    blobSystem.update(registry, config, dtSec);
}

} // namespace

TEST_CASE_METHOD(TestFixture, "BlobAttackSystem spawns area attack with expected components after attack animation")
{
    game::Registry registry;
    game::EnemyAI enemyAI;
    game::PlayerDistanceSystem distanceSystem;
    game::BlobAttackSystem blobSystem;
    config::GameConfig config = makeBlobAttackTestConfig();
    auto locationTable = makeLocationTable(config);

    auto &attackConfig = config.enemyClasses.blob.attack;
    attackConfig.amount = 12.0f;
    attackConfig.pushBackForce = 3.5f;
    attackConfig.stunChance = 0.2f;
    attackConfig.area.radius = 1.0f;
    attackConfig.area.activeTimeSec = 2.5f;
    attackConfig.area.damageTicks = 5;
    attackConfig.area.initialHit = 0.4f;

    config::SpriteConfig areaSpriteConfig = makeSpriteConfig("blob_area.png");
    areaSpriteConfig.texture.size = {20.0f, 30.0f};
    areaSpriteConfig.hitBox.offset = {1.0f, 2.0f};
    areaSpriteConfig.hitBox.size = {6.0f, 8.0f};
    auto &areaIdleState = attackConfig.area.animations.stateToStateConfig[game::AnimationState::Idle];
    areaIdleState.directionToFrames[game::AnimationDirection::None] = {areaSpriteConfig};

    addPlayer(registry, 120.0f, 100.0f);
    const game::Entity enemy = addBlob(registry, 100.0f, 100.0f);

    auto &enemyStats = registry.getComponent<game::EnemyStats>(enemy);
    enemyStats.attackRange = 25.0f;

    runFrame(registry, config, locationTable, enemyAI, distanceSystem, blobSystem, 0.0f);
    REQUIRE(registry.view<game::EnemyAttackTag>().empty());

    runFrame(registry, config, locationTable, enemyAI, distanceSystem, blobSystem, 0.5f);
    REQUIRE(registry.view<game::EnemyAttackTag>().size() == 1);

    const auto attackEntities = registry.view<game::EnemyAttackTag, game::DamageTag, game::Damage, game::HitBox,
                                              game::Position, view::Sprite, game::Animation>();
    REQUIRE(attackEntities.size() == 1);

    const game::Entity areaEntity = attackEntities.front();
    const auto &attackTag = registry.getComponent<game::EnemyAttackTag>(areaEntity);
    const auto &position = registry.getComponent<game::Position>(areaEntity);
    const auto &sprite = registry.getComponent<view::Sprite>(areaEntity);
    const auto &hitBox = registry.getComponent<game::HitBox>(areaEntity);
    const auto &damage = registry.getComponent<game::Damage>(areaEntity);

    REQUIRE(attackTag.source == enemy);
    REQUIRE(position.p.x == Catch::Approx(100.0f));
    REQUIRE(position.p.y == Catch::Approx(100.0f));
    REQUIRE(sprite.imagePath == "blob_area.png");
    REQUIRE(sprite.rect.position.x == Catch::Approx(100.0f));
    REQUIRE(sprite.rect.position.y == Catch::Approx(100.0f));
    REQUIRE(sprite.rect.size.x == Catch::Approx(500.0f));
    REQUIRE(sprite.rect.size.y == Catch::Approx(750.0f));
    REQUIRE(hitBox.offset.x == Catch::Approx(1.0f));
    REQUIRE(hitBox.offset.y == Catch::Approx(2.0f));
    REQUIRE(hitBox.size.x == Catch::Approx(150.0f));
    REQUIRE(hitBox.size.y == Catch::Approx(200.0f));
    REQUIRE(damage.kind == game::DamageKind::Area);
    REQUIRE(damage.amount == Catch::Approx(12.0f));
    REQUIRE(damage.pushBackForce == Catch::Approx(3.5f));
    REQUIRE(damage.stunChance == Catch::Approx(0.2f));
    REQUIRE(std::holds_alternative<game::AreaDamage>(damage.params));

    const auto &areaDamage = std::get<game::AreaDamage>(damage.params);
    REQUIRE(areaDamage.radius == Catch::Approx(1.0f));
    REQUIRE(areaDamage.activeTimeSec == Catch::Approx(2.5f));
    REQUIRE(areaDamage.damageTicks == 5);
    REQUIRE(areaDamage.initialHit == Catch::Approx(0.4f));
    REQUIRE(areaDamage.elapsedSec == Catch::Approx(0.0f));
    REQUIRE(areaDamage.elapsedSecSinceLastTick == Catch::Approx(0.0f));
}

TEST_CASE_METHOD(TestFixture, "BlobAttackSystem does not spawn duplicate area attack in consecutive frames")
{
    game::Registry registry;
    game::EnemyAI enemyAI;
    game::PlayerDistanceSystem distanceSystem;
    game::BlobAttackSystem blobSystem;
    config::GameConfig config = makeBlobAttackTestConfig();
    auto locationTable = makeLocationTable(config);

    auto &attackConfig = config.enemyClasses.blob.attack;
    attackConfig.area.radius = 1.0f;
    attackConfig.area.activeTimeSec = 1.0f;
    attackConfig.area.damageTicks = 2;
    attackConfig.area.initialHit = 0.5f;
    config.enemyClasses.blob.stats.attackSpeed = 1.0f;

    auto &areaIdleState = attackConfig.area.animations.stateToStateConfig[game::AnimationState::Idle];
    areaIdleState.directionToFrames[game::AnimationDirection::None] = {makeSpriteConfig("blob_area.png")};

    addPlayer(registry, 120.0f, 100.0f);
    const game::Entity enemy = addBlob(registry, 100.0f, 100.0f);
    registry.getComponent<game::EnemyStats>(enemy).attackRange = 25.0f;

    runFrame(registry, config, locationTable, enemyAI, distanceSystem, blobSystem, 0.0f);
    runFrame(registry, config, locationTable, enemyAI, distanceSystem, blobSystem, 0.5f);
    runFrame(registry, config, locationTable, enemyAI, distanceSystem, blobSystem, 0.0f);

    const auto spawnedAfterFirstFrame = registry.view<game::EnemyAttackTag>();
    REQUIRE(spawnedAfterFirstFrame.size() == 1);

    runFrame(registry, config, locationTable, enemyAI, distanceSystem, blobSystem, 0.0f);
    const auto spawnedAfterSecondFrame = registry.view<game::EnemyAttackTag>();
    REQUIRE(spawnedAfterSecondFrame.size() == 1);
}

TEST_CASE_METHOD(TestFixture, "BlobAttackSystem area attack respects cooldown before allowing next spawn")
{
    game::Registry registry;
    game::EnemyAI enemyAI;
    game::PlayerDistanceSystem distanceSystem;
    game::BlobAttackSystem blobSystem;
    config::GameConfig config = makeBlobAttackTestConfig();
    auto locationTable = makeLocationTable(config);

    auto &attackConfig = config.enemyClasses.blob.attack;
    attackConfig.area.radius = 1.0f;
    attackConfig.area.activeTimeSec = 1.0f;
    attackConfig.area.damageTicks = 2;
    attackConfig.area.initialHit = 0.5f;
    config.enemyClasses.blob.stats.attackSpeed = 2.0f;

    auto &areaIdleState = attackConfig.area.animations.stateToStateConfig[game::AnimationState::Idle];
    areaIdleState.directionToFrames[game::AnimationDirection::None] = {makeSpriteConfig("blob_area.png")};

    addPlayer(registry, 120.0f, 100.0f);
    const game::Entity enemy = addBlob(registry, 100.0f, 100.0f);
    registry.getComponent<game::EnemyStats>(enemy).attackRange = 25.0f;

    runFrame(registry, config, locationTable, enemyAI, distanceSystem, blobSystem, 0.0f);
    runFrame(registry, config, locationTable, enemyAI, distanceSystem, blobSystem, 0.4f);
    runFrame(registry, config, locationTable, enemyAI, distanceSystem, blobSystem, 0.0f);
    REQUIRE(registry.view<game::EnemyAttackTag>().size() == 1);

    runFrame(registry, config, locationTable, enemyAI, distanceSystem, blobSystem, 0.09f);
    runFrame(registry, config, locationTable, enemyAI, distanceSystem, blobSystem, 0.0f);
    REQUIRE(registry.view<game::EnemyAttackTag>().size() == 1);

    runFrame(registry, config, locationTable, enemyAI, distanceSystem, blobSystem, 0.02f);
    runFrame(registry, config, locationTable, enemyAI, distanceSystem, blobSystem, 0.0f);
    runFrame(registry, config, locationTable, enemyAI, distanceSystem, blobSystem, 0.4f);
    runFrame(registry, config, locationTable, enemyAI, distanceSystem, blobSystem, 0.0f);
    REQUIRE(registry.view<game::EnemyAttackTag>().size() == 2);
}
