#include "controller/debug/debug_context.hpp"
#include "controller/input/input_state.hpp"
#include "controller/view/text.hpp"
#include "game/ecs/components/player_tag.hpp"
#include "game/ecs/components/position.hpp"
#include "game/game.hpp"
#include "shared/util.hpp"

#include <catch2/catch_test_macros.hpp>
#include <variant>

TEST_CASE("Game can be constructed")
{
    REQUIRE_NOTHROW(game::Game{});
}

TEST_CASE("Game update returns false while player is alive")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext debug;

    // ACT
    bool isGameOver = game.update(input, debug, dummyDeltaTime);

    // ASSERT
    REQUIRE_FALSE(isGameOver);
}

TEST_CASE("Game update returns true when no player exists anymore")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext debug;
    debug.active = true;

    game::GameDebugSession &session = game.getDebugSession();
    session.isPlayerDestructionRequested = true;

    // ACT
    bool isGameOver = game.update(input, debug, dummyDeltaTime);

    // ASSERT
    REQUIRE(isGameOver);
}

TEST_CASE("Game update resets stage/wave reload request when debug is active")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext debug;
    debug.active = true;

    game::GameDebugSession &session = game.getDebugSession();
    session.isStageWaveReloadRequested = true;

    // ACT
    bool isGameOver = game.update(input, debug, dummyDeltaTime);

    // ASSERT
    REQUIRE_FALSE(isGameOver);
    REQUIRE_FALSE(session.isStageWaveReloadRequested);
}

TEST_CASE("Game update keeps stage/wave reload request unchanged when debug is inactive")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext debug;
    debug.active = false;

    game::GameDebugSession &session = game.getDebugSession();
    session.isStageWaveReloadRequested = true;

    // ACT
    bool isGameOver = game.update(input, debug, dummyDeltaTime);

    // ASSERT
    REQUIRE_FALSE(isGameOver);
    REQUIRE(session.isStageWaveReloadRequested);
}

TEST_CASE("Game update resets player destruction request when debug is active")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext debug;
    debug.active = true;

    game::GameDebugSession &session = game.getDebugSession();
    session.isPlayerDestructionRequested = true;

    // ACT
    bool isGameOver = game.update(input, debug, dummyDeltaTime);

    // ASSERT
    REQUIRE(isGameOver);
    REQUIRE_FALSE(session.isPlayerDestructionRequested);
}

TEST_CASE("Game update keeps player destruction request unchanged when debug is inactive")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext debug;
    debug.active = false;

    game::GameDebugSession &session = game.getDebugSession();
    session.isPlayerDestructionRequested = true;

    // ACT
    bool isGameOver = game.update(input, debug, dummyDeltaTime);

    // ASSERT
    REQUIRE_FALSE(isGameOver);
    REQUIRE(session.isPlayerDestructionRequested);
}

TEST_CASE("Game update skips system updates when debug is active and system updates are disabled")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext debug;
    debug.active = true;

    game::GameDebugSession &session = game.getDebugSession();
    session.isSystemUpdateActive = false;

    auto players = session.registry.view<game::PlayerTag>();
    REQUIRE_FALSE(players.empty());

    game::Entity player = players.front();
    REQUIRE(session.registry.hasComponent<game::Position>(player));

    game::Position &positionBefore = session.registry.getComponent<game::Position>(player);
    float positionBeforeX = positionBefore.x;
    float positionBeforeY = positionBefore.y;

    input.rightHeld = true;

    // ACT
    bool isGameOver = game.update(input, debug, dummyDeltaTime);

    // ASSERT
    REQUIRE_FALSE(isGameOver);

    game::Position &positionAfter = session.registry.getComponent<game::Position>(player);
    REQUIRE(positionAfter.x == positionBeforeX);
    REQUIRE(positionAfter.y == positionBeforeY);
}

TEST_CASE("Game update still runs system updates when debug is inactive even if system updates are disabled")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext debug;
    debug.active = false;

    game::GameDebugSession &session = game.getDebugSession();
    session.isSystemUpdateActive = false;

    auto players = session.registry.view<game::PlayerTag>();
    REQUIRE_FALSE(players.empty());

    game::Entity player = players.front();
    REQUIRE(session.registry.hasComponent<game::Position>(player));

    game::Position &positionBefore = session.registry.getComponent<game::Position>(player);
    float positionBeforeX = positionBefore.x;
    float positionBeforeY = positionBefore.y;

    input.rightHeld = true;

    // ACT
    bool isGameOver = game.update(input, debug, dummyDeltaTime);

    // ASSERT
    REQUIRE_FALSE(isGameOver);

    game::Position &positionAfter = session.registry.getComponent<game::Position>(player);
    REQUIRE(positionAfter.x != positionBeforeX);
    REQUIRE(positionAfter.y == positionBeforeY);
}

TEST_CASE("Game getView returns an empty view")
{
    // ARRANGE
    game::Game game;

    // ACT
    const controller::View &view = game.getView();

    // ASSERT
    REQUIRE(view.items.empty());
}