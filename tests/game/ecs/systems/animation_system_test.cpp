#include "game/ecs/components/sprite.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/animation_system.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("AnimationSystem sets sprite direction from horizontal velocity")
{
    // ARRANGE
    game::Registry registry;
    game::AnimationSystem system;

    game::Entity e = registry.createEntity();
    registry.addComponent<game::Sprite>(e, {});
    registry.addComponent<game::Velocity>(e, {-50.0f, 0.0f});

    // ACT
    system.update(registry, 0.25f);

    // ASSERT
    const auto &sprite = registry.getComponent<game::Sprite>(e);
    REQUIRE(sprite.direction == game::Direction::Left);
}

TEST_CASE("AnimationSystem advances frame when moving and timer reaches frame duration")
{
    // ARRANGE
    game::Registry registry;
    game::AnimationSystem system;

    game::Entity e = registry.createEntity();
    registry.addComponent<game::Sprite>(e, {game::Direction::Right, 0, 0.0f, 0.5f, 4, "assets/characters/"});
    registry.addComponent<game::Velocity>(e, {100.0f, 0.0f});

    // ACT
    system.update(registry, 0.5f);

    // ASSERT
    const auto &sprite = registry.getComponent<game::Sprite>(e);
    REQUIRE(sprite.currentFrame == 1);
    REQUIRE(sprite.frameTimer == 0.0f);
    REQUIRE(sprite.direction == game::Direction::Right);
}

TEST_CASE("AnimationSystem keeps frame when moving but timer is below frame duration")
{
    // ARRANGE
    game::Registry registry;
    game::AnimationSystem system;

    game::Entity e = registry.createEntity();
    registry.addComponent<game::Sprite>(e, {game::Direction::Right, 2, 0.0f, 0.5f, 4, "assets/characters/"});
    registry.addComponent<game::Velocity>(e, {10.0f, 0.0f});

    // ACT
    system.update(registry, 0.25f);

    // ASSERT
    const auto &sprite = registry.getComponent<game::Sprite>(e);
    REQUIRE(sprite.currentFrame == 2);
    REQUIRE(sprite.frameTimer == 0.25f);
}

TEST_CASE("AnimationSystem resets frame timer when idle")
{
    // ARRANGE
    game::Registry registry;
    game::AnimationSystem system;

    game::Entity e = registry.createEntity();
    registry.addComponent<game::Sprite>(e, {game::Direction::Right, 3, 0.4f, 0.5f, 4, "assets/characters/"});
    registry.addComponent<game::Velocity>(e, {0.0f, 0.0f});

    // ACT
    system.update(registry, 0.25f);

    // ASSERT
    const auto &sprite = registry.getComponent<game::Sprite>(e);
    REQUIRE(sprite.currentFrame == 3);
    REQUIRE(sprite.frameTimer == 0.0f);
}
