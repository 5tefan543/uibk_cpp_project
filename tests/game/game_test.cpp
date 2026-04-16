#include "controller/debug/debug_context.hpp"
#include "controller/input/input_state.hpp"
#include "controller/view/text.hpp"
#include "game/ecs/components/player_tag.hpp"
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

TEST_CASE("Game getView returns at least one text item")
{
    // ARRANGE
    game::Game game;

    // ACT
    controller::View view = game.getView();

    // ASSERT
    REQUIRE(view.items.empty());
}