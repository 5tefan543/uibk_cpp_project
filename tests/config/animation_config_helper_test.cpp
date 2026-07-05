#include "config/animation_config_helper.hpp"
#include "config/game_config.hpp"
#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/stats.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace {

config::SpriteConfig makeSpriteConfig(const std::string &path, float width = 32.0f, float height = 32.0f,
                                      float offsetX = 0.0f, float offsetY = 0.0f, float hitBoxWidth = 32.0f,
                                      float hitBoxHeight = 32.0f)
{
    config::SpriteConfig spriteConfig{};
    spriteConfig.texture.path = path;
    spriteConfig.texture.size = {width, height};
    spriteConfig.hitBox.offset = {offsetX, offsetY};
    spriteConfig.hitBox.size = {hitBoxWidth, hitBoxHeight};
    return spriteConfig;
}

void requireSpriteConfigEquals(const config::SpriteConfig &actual, const config::SpriteConfig &expected)
{
    REQUIRE(actual.texture.path == expected.texture.path);
    REQUIRE(actual.texture.size.x == Catch::Approx(expected.texture.size.x));
    REQUIRE(actual.texture.size.y == Catch::Approx(expected.texture.size.y));

    REQUIRE(actual.hitBox.offset.x == Catch::Approx(expected.hitBox.offset.x));
    REQUIRE(actual.hitBox.offset.y == Catch::Approx(expected.hitBox.offset.y));
    REQUIRE(actual.hitBox.size.x == Catch::Approx(expected.hitBox.size.x));
    REQUIRE(actual.hitBox.size.y == Catch::Approx(expected.hitBox.size.y));
}

void requireFallbackFrame(const config::AnimationFrame &frame, const config::SpriteConfig &fallbackSprite)
{
    requireSpriteConfigEquals(frame.spriteConfig, fallbackSprite);

    REQUIRE(frame.totalFrames == 1);

    // Fallback uses the default values of AnimationStateConfig.
    REQUIRE(frame.frameDuration == Catch::Approx(config::AnimationStateConfig{}.frameDuration));
    REQUIRE(frame.moveSpeedMultiplier == Catch::Approx(config::AnimationStateConfig{}.moveSpeedMultiplier));
}

config::AnimationConfig makeValidAnimationConfig(const std::string &firstFramePath)
{
    config::AnimationConfig animations{};

    animations.stateToStateConfig[game::AnimationState::Walk] = {
        .frameDuration = 0.25f,
        .moveSpeedMultiplier = 0.75f,
        .directionToFrames =
            {
                {
                    game::AnimationDirection::Right,
                    {
                        makeSpriteConfig(firstFramePath, 32.0f, 48.0f, 1.0f, 2.0f, 20.0f, 40.0f),
                        makeSpriteConfig("walk_right_2.png", 33.0f, 49.0f, 3.0f, 4.0f, 21.0f, 41.0f),
                    },
                },
                {
                    game::AnimationDirection::Left,
                    {
                        makeSpriteConfig("walk_left_1.png"),
                    },
                },
            },
    };

    return animations;
}

config::GameConfig makeAnimationHelperTestConfig()
{
    config::GameConfig config{};

    config.fallbackSprite = makeSpriteConfig("fallback.png", 10.0f, 11.0f, 1.0f, 2.0f, 8.0f, 9.0f);

    config.playerClasses.melee.characterType = game::CharacterType::Melee;
    config.playerClasses.melee.animations = makeValidAnimationConfig("melee_walk_right_1.png");

    config.playerClasses.ranged.characterType = game::CharacterType::Ranged;
    config.playerClasses.ranged.animations = makeValidAnimationConfig("ranged_walk_right_1.png");

    config.enemyClasses.blob.enemyType = game::EnemyType::Blob;
    config.enemyClasses.blob.animations = makeValidAnimationConfig("blob_walk_right_1.png");

    config.enemyClasses.boss.enemyType = game::EnemyType::Boss;
    config.enemyClasses.boss.animations = makeValidAnimationConfig("boss_walk_right_1.png");

    config.playerClasses.ranged.attack.projectile.animations = makeValidAnimationConfig("projectile_walk_right_1.png");

    config.playerClasses.ranged.attack.unicorn.animations = makeValidAnimationConfig("unicorn_walk_right_1.png");

    return config;
}

} // namespace

TEST_CASE("AnimationConfigHelper returns configured player animation frame")
{
    const config::GameConfig config = makeAnimationHelperTestConfig();

    const config::AnimationFrame frame = config::AnimationConfigHelper::getPlayerAnimationFrame(
        config, game::CharacterType::Melee, game::AnimationState::Walk, game::AnimationDirection::Right, 1);

    REQUIRE(frame.spriteConfig.texture.path == "walk_right_2.png");
    REQUIRE(frame.spriteConfig.texture.size.x == Catch::Approx(33.0f));
    REQUIRE(frame.spriteConfig.texture.size.y == Catch::Approx(49.0f));

    REQUIRE(frame.spriteConfig.hitBox.offset.x == Catch::Approx(3.0f));
    REQUIRE(frame.spriteConfig.hitBox.offset.y == Catch::Approx(4.0f));
    REQUIRE(frame.spriteConfig.hitBox.size.x == Catch::Approx(21.0f));
    REQUIRE(frame.spriteConfig.hitBox.size.y == Catch::Approx(41.0f));

    REQUIRE(frame.totalFrames == 2);
    REQUIRE(frame.frameDuration == Catch::Approx(0.25f));
    REQUIRE(frame.moveSpeedMultiplier == Catch::Approx(0.75f));
}

TEST_CASE("AnimationConfigHelper selects player animation config by character type")
{
    const config::GameConfig config = makeAnimationHelperTestConfig();

    const config::AnimationFrame meleeFrame = config::AnimationConfigHelper::getPlayerAnimationFrame(
        config, game::CharacterType::Melee, game::AnimationState::Walk, game::AnimationDirection::Right, 0);

    const config::AnimationFrame rangedFrame = config::AnimationConfigHelper::getPlayerAnimationFrame(
        config, game::CharacterType::Ranged, game::AnimationState::Walk, game::AnimationDirection::Right, 0);

    REQUIRE(meleeFrame.spriteConfig.texture.path == "melee_walk_right_1.png");
    REQUIRE(rangedFrame.spriteConfig.texture.path == "ranged_walk_right_1.png");
}

TEST_CASE("AnimationConfigHelper selects enemy animation config by enemy type")
{
    const config::GameConfig config = makeAnimationHelperTestConfig();

    const config::AnimationFrame blobFrame = config::AnimationConfigHelper::getEnemyAnimationFrame(
        config, game::EnemyType::Blob, game::AnimationState::Walk, game::AnimationDirection::Right, 0);

    const config::AnimationFrame bossFrame = config::AnimationConfigHelper::getEnemyAnimationFrame(
        config, game::EnemyType::Boss, game::AnimationState::Walk, game::AnimationDirection::Right, 0);

    REQUIRE(blobFrame.spriteConfig.texture.path == "blob_walk_right_1.png");
    REQUIRE(bossFrame.spriteConfig.texture.path == "boss_walk_right_1.png");
}

TEST_CASE("AnimationConfigHelper returns projectile animation frame")
{
    const config::GameConfig config = makeAnimationHelperTestConfig();

    const config::AnimationFrame frame = config::AnimationConfigHelper::getProjectileAnimationFrame(
        config, config.playerClasses.ranged.attack.projectile, game::AnimationState::Walk,
        game::AnimationDirection::Right, 0);

    REQUIRE(frame.spriteConfig.texture.path == "projectile_walk_right_1.png");
    REQUIRE(frame.totalFrames == 2);
    REQUIRE(frame.frameDuration == Catch::Approx(0.25f));
    REQUIRE(frame.moveSpeedMultiplier == Catch::Approx(0.75f));
}

TEST_CASE("AnimationConfigHelper returns unicorn animation frame")
{
    const config::GameConfig config = makeAnimationHelperTestConfig();

    const config::AnimationFrame frame = config::AnimationConfigHelper::getUnicornAnimationFrame(
        config, config.playerClasses.ranged.attack.unicorn, game::AnimationState::Walk, game::AnimationDirection::Right,
        0);

    REQUIRE(frame.spriteConfig.texture.path == "unicorn_walk_right_1.png");
    REQUIRE(frame.spriteConfig.texture.size.x == Catch::Approx(32.0f));
    REQUIRE(frame.spriteConfig.texture.size.y == Catch::Approx(48.0f));

    REQUIRE(frame.spriteConfig.hitBox.offset.x == Catch::Approx(1.0f));
    REQUIRE(frame.spriteConfig.hitBox.offset.y == Catch::Approx(2.0f));
    REQUIRE(frame.spriteConfig.hitBox.size.x == Catch::Approx(20.0f));
    REQUIRE(frame.spriteConfig.hitBox.size.y == Catch::Approx(40.0f));

    REQUIRE(frame.totalFrames == 2);
    REQUIRE(frame.frameDuration == Catch::Approx(0.25f));
    REQUIRE(frame.moveSpeedMultiplier == Catch::Approx(0.75f));
}

TEST_CASE("AnimationConfigHelper returns selected unicorn animation frame")
{
    const config::GameConfig config = makeAnimationHelperTestConfig();

    const config::AnimationFrame frame = config::AnimationConfigHelper::getUnicornAnimationFrame(
        config, config.playerClasses.ranged.attack.unicorn, game::AnimationState::Walk, game::AnimationDirection::Right,
        1);

    REQUIRE(frame.spriteConfig.texture.path == "walk_right_2.png");
    REQUIRE(frame.spriteConfig.texture.size.x == Catch::Approx(33.0f));
    REQUIRE(frame.spriteConfig.texture.size.y == Catch::Approx(49.0f));

    REQUIRE(frame.spriteConfig.hitBox.offset.x == Catch::Approx(3.0f));
    REQUIRE(frame.spriteConfig.hitBox.offset.y == Catch::Approx(4.0f));
    REQUIRE(frame.spriteConfig.hitBox.size.x == Catch::Approx(21.0f));
    REQUIRE(frame.spriteConfig.hitBox.size.y == Catch::Approx(41.0f));

    REQUIRE(frame.totalFrames == 2);
    REQUIRE(frame.frameDuration == Catch::Approx(0.25f));
    REQUIRE(frame.moveSpeedMultiplier == Catch::Approx(0.75f));
}

TEST_CASE("AnimationConfigHelper returns fallback when animation state is missing")
{
    const config::GameConfig config = makeAnimationHelperTestConfig();

    const config::AnimationFrame frame = config::AnimationConfigHelper::getPlayerAnimationFrame(
        config, game::CharacterType::Melee, game::AnimationState::Attack, game::AnimationDirection::Right, 0);

    requireFallbackFrame(frame, config.fallbackSprite);
}

TEST_CASE("AnimationConfigHelper returns fallback when animation direction is missing")
{
    const config::GameConfig config = makeAnimationHelperTestConfig();

    const config::AnimationFrame frame = config::AnimationConfigHelper::getPlayerAnimationFrame(
        config, game::CharacterType::Melee, game::AnimationState::Walk, game::AnimationDirection::Up, 0);

    requireFallbackFrame(frame, config.fallbackSprite);
}

TEST_CASE("AnimationConfigHelper returns fallback when animation frame vector is empty")
{
    config::GameConfig config = makeAnimationHelperTestConfig();

    config.playerClasses.melee.animations.stateToStateConfig[game::AnimationState::Walk]
        .directionToFrames[game::AnimationDirection::Right]
        .clear();

    const config::AnimationFrame frame = config::AnimationConfigHelper::getPlayerAnimationFrame(
        config, game::CharacterType::Melee, game::AnimationState::Walk, game::AnimationDirection::Right, 0);

    requireFallbackFrame(frame, config.fallbackSprite);
}

TEST_CASE("AnimationConfigHelper returns fallback when frame number is out of bounds")
{
    const config::GameConfig config = makeAnimationHelperTestConfig();

    const config::AnimationFrame frame = config::AnimationConfigHelper::getPlayerAnimationFrame(
        config, game::CharacterType::Melee, game::AnimationState::Walk, game::AnimationDirection::Right, 2);

    requireFallbackFrame(frame, config.fallbackSprite);
}

TEST_CASE("AnimationConfigHelper returns fallback through enemy wrapper")
{
    const config::GameConfig config = makeAnimationHelperTestConfig();

    const config::AnimationFrame frame = config::AnimationConfigHelper::getEnemyAnimationFrame(
        config, game::EnemyType::Blob, game::AnimationState::Death, game::AnimationDirection::Right, 0);

    requireFallbackFrame(frame, config.fallbackSprite);
}

TEST_CASE("AnimationConfigHelper returns fallback through projectile wrapper")
{
    const config::GameConfig config = makeAnimationHelperTestConfig();

    const config::AnimationFrame frame = config::AnimationConfigHelper::getProjectileAnimationFrame(
        config, config.playerClasses.ranged.attack.projectile, game::AnimationState::Death,
        game::AnimationDirection::Right, 0);

    requireFallbackFrame(frame, config.fallbackSprite);
}

TEST_CASE("AnimationConfigHelper returns fallback through unicorn wrapper")
{
    const config::GameConfig config = makeAnimationHelperTestConfig();

    const config::AnimationFrame frame = config::AnimationConfigHelper::getUnicornAnimationFrame(
        config, config.playerClasses.ranged.attack.unicorn, game::AnimationState::Death,
        game::AnimationDirection::Right, 0);

    requireFallbackFrame(frame, config.fallbackSprite);
}