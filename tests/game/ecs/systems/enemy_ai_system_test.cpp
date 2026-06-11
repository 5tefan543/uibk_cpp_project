#include "game/ecs/components/enemy_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/ecs/components/velocity.hpp"
#include "game/ecs/registry.hpp"
#include "game/ecs/systems/movement_system.hpp"
#include "shared/test_fixture.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE_METHOD(TestFixture, "Enemies run towards player")
{
    game::Registry registry;
    game::Entity player = registry.createEntity();
    registry.addComponent<game::Position>(player, {25.0f, 0.0f});
    registry.addComponent<game::Velocity>(player, {0.0f, 0.0f});
    registry.addComponent(player, game::PlayerStats{});

    game::Entity e1 = registry.createEntity();
    registry.addComponent<game::Position>(e1, {-50.0f, 0.0f});
    registry.addComponent<game::Velocity>(e1, {-1.0f, -1.0f});
    game::Entity e2 = registry.createEntity();
    registry.addComponent<game::Position>(e2, {50.0f, 0.0f});
    registry.addComponent<game::Velocity>(e2, {1.0f, 1.0f});

    game::EnemyStats eStats;
    eStats.moveSpeed = 10;
    view::Sprite eSprite;
    eSprite.height = 10;
    eSprite.width = 10;
    for (game::Entity e : {e1, e2}) {
        registry.addComponent<game::EnemyStats>(e, eStats);
        registry.addComponent<view::Sprite>(e, eSprite);
        registry.addComponent<game::EnemyTag>(e, {});
    }

    game::LocationTable locTab({5, 5}, {100, 100});
    game::EnemyAI ai;
    game::MovementSystem movSys;

    for (unsigned i = 0; i < 6; i++) {
        locTab.update(registry);
        ai.update(registry, locTab);
        movSys.update(registry, 2);
    }

    auto pPos = registry.getComponent<game::Position>(player);
    for (game::Entity e : {e1, e2}) {
        auto ePos = registry.getComponent<game::Position>(e);
        REQUIRE(std::abs(ePos.x - pPos.x) < 6);
        REQUIRE(std::abs(ePos.y - pPos.y) < 6);
    }
}
