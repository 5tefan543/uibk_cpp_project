#include "config/game_config.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/stats.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/enemy_ai_system.hpp"
#include "game/location_table.hpp"
#include "shared/test_fixture.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

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

config::GameConfig makeEnemyAITestConfig()
{
    config::GameConfig config{};

    config.mapConfig.mapSize = {1000.0f, 1000.0f};
    config.locationTableConfig.numBuckets = {10, 10};

    config.enemyClasses.blob.enemyType = game::EnemyType::Blob;

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

    return config;
}

game::LocationTable makeLocationTable(const config::GameConfig &config)
{
    // Adjust this line if your constructor has a different argument order.
    return game::LocationTable{
        config.locationTableConfig.numBuckets,
        config.mapConfig.mapSize,
    };
}

game::Entity addPlayer(game::Registry &registry, float x, float y)
{
    const game::Entity player = registry.createEntity();

    registry.addComponent<game::PlayerTag>(player, {});
    registry.addComponent<game::Position>(player, {x, y});

    return player;
}

game::EnemyStats makeEnemyStats(float moveSpeed = 100.0f)
{
    game::EnemyStats stats{};
    stats.enemyType = game::EnemyType::Blob;
    stats.moveSpeed = moveSpeed;
    return stats;
}

game::Animation makeEnemyAnimation(game::AnimationState state = game::AnimationState::Idle,
                                   game::AnimationDirection direction = game::AnimationDirection::Right)
{
    game::Animation animation{};
    animation.state = state;
    animation.direction = direction;
    animation.currentFrame = 0;
    animation.frameTimer = 0.0f;
    animation.stateTimeRemaining = 0.0f;
    return animation;
}

game::Entity addEnemy(game::Registry &registry, float x, float y, float moveSpeed = 100.0f,
                      game::AnimationState state = game::AnimationState::Idle,
                      game::AnimationDirection direction = game::AnimationDirection::Right)
{
    const game::Entity enemy = registry.createEntity();

    registry.addComponent<game::EnemyTag>(enemy, {});
    registry.addComponent<game::Position>(enemy, {x, y});
    registry.addComponent<game::Velocity>(enemy, {});
    registry.addComponent<game::EnemyStats>(enemy, makeEnemyStats(moveSpeed));
    registry.addComponent<game::Animation>(enemy, makeEnemyAnimation(state, direction));

    return enemy;
}

} // namespace

TEST_CASE_METHOD(TestFixture, "EnemyAI does not update enemies when no player exists")
{
    game::Registry registry;
    game::EnemyAI system;
    const config::GameConfig config = makeEnemyAITestConfig();
    auto locationTable = makeLocationTable(config);

    const game::Entity enemy = addEnemy(registry, 100.0f, 100.0f);

    auto &velocityBefore = registry.getComponent<game::Velocity>(enemy).v;
    velocityBefore = {12.0f, 34.0f};

    auto &animationBefore = registry.getComponent<game::Animation>(enemy);
    animationBefore.state = game::AnimationState::Idle;
    animationBefore.direction = game::AnimationDirection::Left;
    animationBefore.currentFrame = 1;
    animationBefore.frameTimer = 0.15f;

    system.update(registry, config, locationTable, 0.1f);

    const auto &velocity = registry.getComponent<game::Velocity>(enemy).v;
    const auto &animation = registry.getComponent<game::Animation>(enemy);

    REQUIRE(velocity.x == Catch::Approx(12.0f));
    REQUIRE(velocity.y == Catch::Approx(34.0f));

    REQUIRE(animation.state == game::AnimationState::Idle);
    REQUIRE(animation.direction == game::AnimationDirection::Left);
    REQUIRE(animation.currentFrame == 1);
    REQUIRE(animation.frameTimer == Catch::Approx(0.15f));
}

TEST_CASE_METHOD(TestFixture, "EnemyAI moves enemy directly toward player on horizontal axis")
{
    game::Registry registry;
    game::EnemyAI system;
    const config::GameConfig config = makeEnemyAITestConfig();
    auto locationTable = makeLocationTable(config);

    addPlayer(registry, 200.0f, 100.0f);
    const game::Entity enemy = addEnemy(registry, 100.0f, 100.0f, 100.0f);

    system.update(registry, config, locationTable, 0.0f);

    const auto &velocity = registry.getComponent<game::Velocity>(enemy).v;
    const auto &animation = registry.getComponent<game::Animation>(enemy);

    REQUIRE(velocity.x == Catch::Approx(100.0f));
    REQUIRE(velocity.y == Catch::Approx(0.0f));

    REQUIRE(animation.state == game::AnimationState::Walk);
    REQUIRE(animation.direction == game::AnimationDirection::Right);
}

TEST_CASE_METHOD(TestFixture, "EnemyAI normalizes diagonal movement toward player")
{
    game::Registry registry;
    game::EnemyAI system;
    const config::GameConfig config = makeEnemyAITestConfig();
    auto locationTable = makeLocationTable(config);

    addPlayer(registry, 200.0f, 200.0f);
    const game::Entity enemy = addEnemy(registry, 100.0f, 100.0f, 100.0f);

    system.update(registry, config, locationTable, 0.0f);

    const auto &velocity = registry.getComponent<game::Velocity>(enemy).v;

    REQUIRE(velocity.x == Catch::Approx(70.71067f));
    REQUIRE(velocity.y == Catch::Approx(70.71067f));
}

TEST_CASE_METHOD(TestFixture, "EnemyAI stops enemy when already very close to player")
{
    game::Registry registry;
    game::EnemyAI system;
    const config::GameConfig config = makeEnemyAITestConfig();
    auto locationTable = makeLocationTable(config);

    addPlayer(registry, 100.0f, 100.0f);
    const game::Entity enemy =
        addEnemy(registry, 103.0f, 100.0f, 100.0f, game::AnimationState::Walk, game::AnimationDirection::Right);

    auto &velocityBefore = registry.getComponent<game::Velocity>(enemy).v;
    velocityBefore = {50.0f, 0.0f};

    system.update(registry, config, locationTable, 0.0f);

    const auto &velocity = registry.getComponent<game::Velocity>(enemy).v;
    const auto &animation = registry.getComponent<game::Animation>(enemy);

    REQUIRE(velocity.x == Catch::Approx(0.0f));
    REQUIRE(velocity.y == Catch::Approx(0.0f));

    REQUIRE(animation.state == game::AnimationState::Idle);
    REQUIRE(animation.direction == game::AnimationDirection::Right);
}

TEST_CASE_METHOD(TestFixture, "EnemyAI changes animation direction to left when moving left")
{
    game::Registry registry;
    game::EnemyAI system;
    const config::GameConfig config = makeEnemyAITestConfig();
    auto locationTable = makeLocationTable(config);

    addPlayer(registry, 0.0f, 100.0f);
    const game::Entity enemy =
        addEnemy(registry, 100.0f, 100.0f, 100.0f, game::AnimationState::Idle, game::AnimationDirection::Right);

    system.update(registry, config, locationTable, 0.0f);

    const auto &velocity = registry.getComponent<game::Velocity>(enemy).v;
    const auto &animation = registry.getComponent<game::Animation>(enemy);

    REQUIRE(velocity.x == Catch::Approx(-100.0f));
    REQUIRE(velocity.y == Catch::Approx(0.0f));

    REQUIRE(animation.state == game::AnimationState::Walk);
    REQUIRE(animation.direction == game::AnimationDirection::Left);
}

TEST_CASE_METHOD(TestFixture, "EnemyAI resets animation frame when animation state changes")
{
    game::Registry registry;
    game::EnemyAI system;
    const config::GameConfig config = makeEnemyAITestConfig();
    auto locationTable = makeLocationTable(config);

    addPlayer(registry, 200.0f, 100.0f);
    const game::Entity enemy =
        addEnemy(registry, 100.0f, 100.0f, 100.0f, game::AnimationState::Idle, game::AnimationDirection::Right);

    auto &animationBefore = registry.getComponent<game::Animation>(enemy);
    animationBefore.currentFrame = 1;
    animationBefore.frameTimer = 0.15f;

    system.update(registry, config, locationTable, 0.0f);

    const auto &animation = registry.getComponent<game::Animation>(enemy);

    REQUIRE(animation.state == game::AnimationState::Walk);
    REQUIRE(animation.direction == game::AnimationDirection::Right);
    REQUIRE(animation.currentFrame == 0);
    REQUIRE(animation.frameTimer == Catch::Approx(0.0f));
}

TEST_CASE_METHOD(TestFixture, "EnemyAI keeps animation frame when state and direction stay unchanged")
{
    game::Registry registry;
    game::EnemyAI system;
    const config::GameConfig config = makeEnemyAITestConfig();
    auto locationTable = makeLocationTable(config);

    addPlayer(registry, 200.0f, 100.0f);
    const game::Entity enemy =
        addEnemy(registry, 100.0f, 100.0f, 100.0f, game::AnimationState::Walk, game::AnimationDirection::Right);

    auto &animationBefore = registry.getComponent<game::Animation>(enemy);
    animationBefore.currentFrame = 1;
    animationBefore.frameTimer = 0.15f;

    system.update(registry, config, locationTable, 0.0f);

    const auto &animation = registry.getComponent<game::Animation>(enemy);

    REQUIRE(animation.state == game::AnimationState::Walk);
    REQUIRE(animation.direction == game::AnimationDirection::Right);
    REQUIRE(animation.currentFrame == 1);
    REQUIRE(animation.frameTimer == Catch::Approx(0.15f));
}

TEST_CASE_METHOD(TestFixture, "EnemyAI does not replace timed animation while time remains")
{
    game::Registry registry;
    game::EnemyAI system;
    const config::GameConfig config = makeEnemyAITestConfig();
    auto locationTable = makeLocationTable(config);

    addPlayer(registry, 200.0f, 100.0f);
    const game::Entity enemy =
        addEnemy(registry, 100.0f, 100.0f, 100.0f, game::AnimationState::Attack, game::AnimationDirection::Left);

    auto &animationBefore = registry.getComponent<game::Animation>(enemy);
    animationBefore.stateTimeRemaining = 0.5f;

    system.update(registry, config, locationTable, 0.1f);

    const auto &animation = registry.getComponent<game::Animation>(enemy);
    const auto &velocity = registry.getComponent<game::Velocity>(enemy).v;

    REQUIRE(velocity.x == Catch::Approx(100.0f));
    REQUIRE(velocity.y == Catch::Approx(0.0f));

    REQUIRE(animation.state == game::AnimationState::Attack);
    REQUIRE(animation.direction == game::AnimationDirection::Left);
    REQUIRE(animation.stateTimeRemaining == Catch::Approx(0.4f));
}

TEST_CASE_METHOD(TestFixture, "EnemyAI applies animation move speed multiplier from enemy animation config")
{
    game::Registry registry;
    game::EnemyAI system;
    config::GameConfig config = makeEnemyAITestConfig();

    config.enemyClasses.blob.animations.stateToStateConfig[game::AnimationState::Walk].moveSpeedMultiplier = 0.5f;

    auto locationTable = makeLocationTable(config);

    addPlayer(registry, 200.0f, 100.0f);
    const game::Entity enemy = addEnemy(registry, 100.0f, 100.0f, 100.0f);

    system.update(registry, config, locationTable, 0.0f);

    const auto &velocity = registry.getComponent<game::Velocity>(enemy).v;
    const auto &animation = registry.getComponent<game::Animation>(enemy);

    REQUIRE(animation.state == game::AnimationState::Walk);
    REQUIRE(animation.direction == game::AnimationDirection::Right);

    REQUIRE(velocity.x == Catch::Approx(50.0f));
    REQUIRE(velocity.y == Catch::Approx(0.0f));
}

TEST_CASE_METHOD(TestFixture, "EnemyAI leaves enemy unchanged when move speed is zero")
{
    game::Registry registry;
    game::EnemyAI system;
    const config::GameConfig config = makeEnemyAITestConfig();
    auto locationTable = makeLocationTable(config);

    addPlayer(registry, 200.0f, 100.0f);
    const game::Entity enemy =
        addEnemy(registry, 100.0f, 100.0f, 0.0f, game::AnimationState::Idle, game::AnimationDirection::Right);

    auto &velocityBefore = registry.getComponent<game::Velocity>(enemy);
    velocityBefore = {12.0f, 34.0f};

    system.update(registry, config, locationTable, 0.0f);

    const auto &velocity = registry.getComponent<game::Velocity>(enemy).v;
    const auto &animation = registry.getComponent<game::Animation>(enemy);

    REQUIRE(velocity.x == Catch::Approx(0.0f));
    REQUIRE(velocity.y == Catch::Approx(0.0f));
    REQUIRE(animation.state == game::AnimationState::Idle);
}