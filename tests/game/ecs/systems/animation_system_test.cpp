#include "game/ecs/components/animation.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/animation_system.hpp"
#include "shared/test_fixture.hpp"
#include "view/sprite.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE_METHOD(TestFixture, "AnimationSystem sets sprite direction from horizontal velocity")
{
    game::Registry registry;
    game::AnimationSystem system;

    game::Entity e = registry.createEntity();
    registry.addComponent<game::Animation>(e, {.direction = game::Direction::Right,
                                               .currentFrame = 0,
                                               .frameTimer = 0.0f,
                                               .frameDuration = 0.5f,
                                               .totalFrames = 4,
                                               .baseTexturePath = "assets/characters/"});
    registry.addComponent<view::Sprite>(e, {});
    registry.addComponent<game::Velocity>(e, {-50.0f, 0.0f});

    system.update(registry, 0.1f);

    const auto &animation = registry.getComponent<game::Animation>(e);
    const auto &sprite = registry.getComponent<view::Sprite>(e);

    REQUIRE(animation.direction == game::Direction::Left);
    REQUIRE(sprite.imagePath == "assets/characters/left_1.png");
}

TEST_CASE_METHOD(TestFixture, "AnimationSystem advances frame when moving and timer reaches frame duration")
{
    game::Registry registry;
    game::AnimationSystem system;

    game::Entity e = registry.createEntity();
    registry.addComponent<game::Animation>(e, {.direction = game::Direction::Right,
                                               .currentFrame = 0,
                                               .frameTimer = 0.0f,
                                               .frameDuration = 0.5f,
                                               .totalFrames = 4,
                                               .baseTexturePath = "assets/characters/"});
    registry.addComponent<view::Sprite>(e, {});
    registry.addComponent<game::Velocity>(e, {100.0f, 0.0f});

    system.update(registry, 0.5f);

    const auto &animation = registry.getComponent<game::Animation>(e);
    const auto &sprite = registry.getComponent<view::Sprite>(e);

    REQUIRE(animation.currentFrame == 1);
    REQUIRE(animation.frameTimer == 0.0f);
    REQUIRE(animation.direction == game::Direction::Right);
    REQUIRE(sprite.imagePath == "assets/characters/right_2.png");
}

TEST_CASE_METHOD(TestFixture, "AnimationSystem keeps frame when moving but timer is below frame duration")
{
    game::Registry registry;
    game::AnimationSystem system;

    game::Entity e = registry.createEntity();
    registry.addComponent<game::Animation>(e, {.direction = game::Direction::Right,
                                               .currentFrame = 2,
                                               .frameTimer = 0.0f,
                                               .frameDuration = 0.5f,
                                               .totalFrames = 4,
                                               .baseTexturePath = "assets/characters/"});
    registry.addComponent<view::Sprite>(e, {});
    registry.addComponent<game::Velocity>(e, {10.0f, 0.0f});

    system.update(registry, 0.25f);

    const auto &animation = registry.getComponent<game::Animation>(e);
    const auto &sprite = registry.getComponent<view::Sprite>(e);

    REQUIRE(animation.currentFrame == 2);
    REQUIRE(animation.frameTimer == 0.25f);
    REQUIRE(sprite.imagePath == "assets/characters/right_3.png");
}

TEST_CASE_METHOD(TestFixture, "AnimationSystem resets frame timer when idle")
{
    game::Registry registry;
    game::AnimationSystem system;

    game::Entity e = registry.createEntity();
    registry.addComponent<game::Animation>(e, {.direction = game::Direction::Right,
                                               .currentFrame = 3,
                                               .frameTimer = 0.4f,
                                               .frameDuration = 0.5f,
                                               .totalFrames = 4,
                                               .baseTexturePath = "assets/characters/"});
    registry.addComponent<view::Sprite>(e, {});
    registry.addComponent<game::Velocity>(e, {0.0f, 0.0f});

    system.update(registry, 0.25f);

    const auto &animation = registry.getComponent<game::Animation>(e);
    const auto &sprite = registry.getComponent<view::Sprite>(e);

    REQUIRE(animation.currentFrame == 3);
    REQUIRE(animation.frameTimer == 0.0f);
    REQUIRE(sprite.imagePath == "assets/characters/right_4.png");
}