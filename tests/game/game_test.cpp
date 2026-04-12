#include "controller/input/input_state.hpp"
#include "controller/state/state_transition_action.hpp"
#include "controller/view/text.hpp"
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

    // ACT & ASSERT
    REQUIRE(game.update(input, dummyDeltaTime) == controller::StateTransitionAction::PushPauseMenu);
}

TEST_CASE("Game update returns PushProgressionStore on mouseLeftPressed")
{
    // ARRANGE
    game::Game game;

    controller::InputState input;
    input.mouseLeftPressed = true;

    // ACT & ASSERT
    REQUIRE(game.update(input, dummyDeltaTime) == controller::StateTransitionAction::PushProgressionStore);
}

TEST_CASE("Game update returns ReplaceCurrentWithGameOverMenu on confirmPressed")
{
    // ARRANGE
    game::Game game;

    controller::InputState input;
    input.confirmPressed = true;

    // ACT & ASSERT
    REQUIRE(game.update(input, dummyDeltaTime) == controller::StateTransitionAction::ReplaceCurrentWithGameOverMenu);
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