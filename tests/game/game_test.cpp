#include "controller/input/input_state.hpp"
#include "controller/state/state_transition_action.hpp"
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

TEST_CASE("Game update returns PushPauseMenu on cancelPressed")
{
    // ARRANGE
    game::Game game;

    controller::InputState input;
    input.cancelPressed = true;
    controller::DebugContext debug;

    // ACT & ASSERT
    REQUIRE(game.update(input, debug, dummyDeltaTime) == controller::StateTransitionAction::PushPauseMenu);
}

TEST_CASE("Game update returns PushProgressionStore on mouseLeftPressed")
{
    // ARRANGE
    game::Game game;

    controller::InputState input;
    input.mouseLeftPressed = true;
    controller::DebugContext debug;

    // ACT & ASSERT
    REQUIRE(game.update(input, debug, dummyDeltaTime) == controller::StateTransitionAction::PushProgressionStore);
}

TEST_CASE("Game update returns ReplaceCurrentWithGameOverMenu on confirmPressed")
{
    // ARRANGE
    game::Game game;

    controller::InputState input;
    input.confirmPressed = true;
    controller::DebugContext debug;

    // ACT & ASSERT
    REQUIRE(game.update(input, debug, dummyDeltaTime)
            == controller::StateTransitionAction::ReplaceCurrentWithGameOverMenu);
}

TEST_CASE("Game getView returns at least one text item")
{
    // ARRANGE
    game::Game game;

    // ACT
    controller::View view = game.getView();

    // ASSERT
    REQUIRE_FALSE(view.items.empty());
    REQUIRE(std::holds_alternative<controller::Text>(view.items[0]));
}

TEST_CASE("Game update sets GameDebugSession when action is not game over")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext debug;

    // PRECONDITION
    REQUIRE(debug.gameSession == nullptr);

    // ACT
    controller::StateTransitionAction action = game.update(input, debug, dummyDeltaTime);

    // ASSERT
    REQUIRE(action == controller::StateTransitionAction::None);
    REQUIRE(debug.gameSession != nullptr);
}

TEST_CASE("Game update resets GameDebugSession on game over")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    input.confirmPressed = true;

    controller::DebugContext debug;

    // ACT
    controller::StateTransitionAction action = game.update(input, debug, dummyDeltaTime);

    // ASSERT
    REQUIRE(action == controller::StateTransitionAction::ReplaceCurrentWithGameOverMenu);
    REQUIRE(debug.gameSession == nullptr);
}

TEST_CASE("Game update resets GameDebugSession stage/wave reload request when debug is active")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext debug;
    debug.active = true;
    debug.gameSettings.stage = 2;
    debug.gameSettings.wave = 3;

    // update needs to be executed once such that gameSession is set
    game.update(input, debug, dummyDeltaTime);
    REQUIRE(debug.gameSession != nullptr);

    // now request the stage/wave reload
    debug.gameSession->isStageWaveReloadRequested = true;

    // ACT
    controller::StateTransitionAction action = game.update(input, debug, dummyDeltaTime);

    // ASSERT
    REQUIRE(action == controller::StateTransitionAction::None);
    REQUIRE_FALSE(debug.gameSession->isStageWaveReloadRequested);
}

TEST_CASE("Game update keeps GameDebugSession stage/wave reload request unchanged when debug is inactive")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugContext debug;
    debug.active = false;

    // update needs to be executed once such that gameSession is set
    game.update(input, debug, dummyDeltaTime);
    REQUIRE(debug.gameSession != nullptr);

    // now request the stage/wave reload
    debug.gameSession->isStageWaveReloadRequested = true;

    // ACT
    controller::StateTransitionAction action = game.update(input, debug, dummyDeltaTime);

    // ASSERT
    REQUIRE(action == controller::StateTransitionAction::None);
    REQUIRE(debug.gameSession->isStageWaveReloadRequested);
}