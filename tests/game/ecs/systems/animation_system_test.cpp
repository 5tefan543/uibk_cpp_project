// #include "game/ecs/components/animation.hpp"
// #include "game/ecs/components/velocity.hpp"
// #include "game/ecs/registry.hpp"
// #include "game/ecs/systems/animation_system.hpp"
// #include "shared/test_fixture.hpp"
// #include "view/sprite.hpp"

// #include <catch2/catch_test_macros.hpp>

// TEST_CASE_METHOD(TestFixture, "AnimationSystem sets sprite direction from horizontal velocity")
// {
//     game::Registry registry;
//     game::AnimationSystem system;

//     game::Entity e = registry.createEntity();
//     registry.addComponent<game::Animation>(e, {.direction = game::AnimationDirection::Right,
//                                                .currentFrame = 0,
//                                                .frameTimer = 0.0f,
//                                                .frameDuration = 0.5f,
//                                                .totalFrames = 4,
//                                                .baseTexturePath = "assets/characters/"});
//     registry.addComponent<view::Sprite>(e, {});
//     registry.addComponent<game::Velocity>(e, {-50.0f, 0.0f});

//     system.update(registry, config 0.1f);

//     const auto &animation = registry.getComponent<game::Animation>(e);
//     const auto &sprite = registry.getComponent<view::Sprite>(e);

//     REQUIRE(animation.direction == game::AnimationDirection::Left);
//     REQUIRE(sprite.imagePath == "assets/characters/left_1.png");
// }

// TEST_CASE_METHOD(TestFixture, "AnimationSystem advances frame when moving and timer reaches frame duration")
// {
//     game::Registry registry;
//     game::AnimationSystem system;

//     game::Entity e = registry.createEntity();
//     registry.addComponent<game::Animation>(e, {.direction = game::AnimationDirection::Right,
//                                                .currentFrame = 0,
//                                                .frameTimer = 0.0f,
//                                                .frameDuration = 0.5f,
//                                                .totalFrames = 4,
//                                                .baseTexturePath = "assets/characters/"});
//     registry.addComponent<view::Sprite>(e, {});
//     registry.addComponent<game::Velocity>(e, {100.0f, 0.0f});

//     system.update(registry, 0.5f);

//     const auto &animation = registry.getComponent<game::Animation>(e);
//     const auto &sprite = registry.getComponent<view::Sprite>(e);

//     REQUIRE(animation.currentFrame == 1);
//     REQUIRE(animation.frameTimer == 0.0f);
//     REQUIRE(animation.direction == game::AnimationDirection::Right);
//     REQUIRE(sprite.imagePath == "assets/characters/right_2.png");
// }

// TEST_CASE_METHOD(TestFixture, "AnimationSystem keeps frame when moving but timer is below frame duration")
// {
//     game::Registry registry;
//     game::AnimationSystem system;

//     game::Entity e = registry.createEntity();
//     registry.addComponent<game::Animation>(e, {.direction = game::AnimationDirection::Right,
//                                                .currentFrame = 2,
//                                                .frameTimer = 0.0f,
//                                                .frameDuration = 0.5f,
//                                                .totalFrames = 4,
//                                                .baseTexturePath = "assets/characters/"});
//     registry.addComponent<view::Sprite>(e, {});
//     registry.addComponent<game::Velocity>(e, {10.0f, 0.0f});

//     system.update(registry, 0.25f);

//     const auto &animation = registry.getComponent<game::Animation>(e);
//     const auto &sprite = registry.getComponent<view::Sprite>(e);

//     REQUIRE(animation.currentFrame == 2);
//     REQUIRE(animation.frameTimer == 0.25f);
//     REQUIRE(sprite.imagePath == "assets/characters/right_3.png");
// }

// TEST_CASE_METHOD(TestFixture, "AnimationSystem resets frame timer when idle")
// {
//     game::Registry registry;
//     game::AnimationSystem system;

//     game::Entity e = registry.createEntity();
//     registry.addComponent<game::Animation>(e, {.direction = game::AnimationDirection::Right,
//                                                .currentFrame = 3,
//                                                .frameTimer = 0.4f,
//                                                .frameDuration = 0.5f,
//                                                .totalFrames = 4,
//                                                .baseTexturePath = "assets/characters/"});
//     registry.addComponent<view::Sprite>(e, {});
//     registry.addComponent<game::Velocity>(e, {0.0f, 0.0f});

//     system.update(registry, 0.25f);

//     const auto &animation = registry.getComponent<game::Animation>(e);
//     const auto &sprite = registry.getComponent<view::Sprite>(e);

//     REQUIRE(animation.currentFrame == 3);
//     REQUIRE(animation.frameTimer == 0.0f);
//     REQUIRE(sprite.imagePath == "assets/characters/right_4.png");
// }

// TEST_CASE_METHOD(TestFixture, "AnimationSystem prioritizes attack override sprite and direction")
// {
//     game::Registry registry;
//     game::AnimationSystem system;

//     game::Entity e = registry.createEntity();
//     registry.addComponent<game::Animation>(e, {.direction = game::AnimationDirection::Right,
//                                                .currentFrame = 0,
//                                                .frameTimer = 0.0f,
//                                                .frameDuration = 0.5f,
//                                                .totalFrames = 4,
//                                                .baseTexturePath = "assets/characters/",
//                                                .overrideState = game::AnimationOverrideState::Attack,
//                                                .overrideTimeRemaining = 0.6f,
//                                                .overrideDirection = game::AnimationDirection::Left,
//                                                .attackTexturePath = "assets/characters/atk_",
//                                                .attackFrameDuration = 0.3f,
//                                                .attackTotalFrames = 2});
//     registry.addComponent<view::Sprite>(e, {});
//     registry.addComponent<game::Velocity>(e, {100.0f, 0.0f});

//     system.update(registry, 0.3f);

//     const auto &animation = registry.getComponent<game::Animation>(e);
//     const auto &sprite = registry.getComponent<view::Sprite>(e);

//     REQUIRE(animation.currentFrame == 1);
//     REQUIRE(animation.overrideState == game::AnimationOverrideState::Attack);
//     REQUIRE(animation.direction == game::AnimationDirection::Left);
//     REQUIRE(sprite.imagePath == "assets/characters/atk_left_2.png");
// }

// TEST_CASE_METHOD(TestFixture, "AnimationSystem prioritizes death override sprite and direction")
// {
//     game::Registry registry;
//     game::AnimationSystem system;

//     game::Entity e = registry.createEntity();
//     registry.addComponent<game::Animation>(e, {.direction = game::AnimationDirection::Right,
//                                                .currentFrame = 0,
//                                                .frameTimer = 0.0f,
//                                                .frameDuration = 0.5f,
//                                                .totalFrames = 4,
//                                                .baseTexturePath = "assets/characters/",
//                                                .overrideState = game::AnimationOverrideState::Death,
//                                                .overrideTimeRemaining = 0.6f,
//                                                .overrideDirection = game::AnimationDirection::Left,
//                                                .deathTexturePath = "assets/characters/death_",
//                                                .deathFrameDuration = 0.3f,
//                                                .deathTotalFrames = 2});
//     registry.addComponent<view::Sprite>(e, {});
//     registry.addComponent<game::Velocity>(e, {100.0f, 0.0f});

//     system.update(registry, 0.3f);

//     const auto &animation = registry.getComponent<game::Animation>(e);
//     const auto &sprite = registry.getComponent<view::Sprite>(e);

//     REQUIRE(animation.currentFrame == 1);
//     REQUIRE(animation.overrideState == game::AnimationOverrideState::Death);
//     REQUIRE(animation.direction == game::AnimationDirection::Left);
//     REQUIRE(sprite.imagePath == "assets/characters/death_left_2.png");
// }

// TEST_CASE_METHOD(TestFixture, "AnimationSystem clears attack override after timer expires")
// {
//     game::Registry registry;
//     game::AnimationSystem system;

//     game::Entity e = registry.createEntity();
//     registry.addComponent<game::Animation>(e, {.direction = game::AnimationDirection::Right,
//                                                .currentFrame = 0,
//                                                .frameTimer = 0.0f,
//                                                .frameDuration = 0.5f,
//                                                .totalFrames = 4,
//                                                .baseTexturePath = "assets/characters/",
//                                                .overrideState = game::AnimationOverrideState::Attack,
//                                                .overrideTimeRemaining = 0.2f,
//                                                .overrideDirection = game::AnimationDirection::Right,
//                                                .attackTexturePath = "assets/characters/atk_",
//                                                .attackFrameDuration = 0.2f,
//                                                .attackTotalFrames = 2});
//     registry.addComponent<view::Sprite>(e, {});
//     registry.addComponent<game::Velocity>(e, {0.0f, 0.0f});

//     system.update(registry, 0.2f);

//     const auto &animation = registry.getComponent<game::Animation>(e);
//     REQUIRE(animation.overrideState == game::AnimationOverrideState::None);
//     REQUIRE(animation.currentFrame == 0);

//     system.update(registry, 0.0f);
//     const auto &sprite = registry.getComponent<view::Sprite>(e);
//     REQUIRE(sprite.imagePath == "assets/characters/right_1.png");
// }