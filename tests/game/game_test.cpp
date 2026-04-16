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
    controller::DebugState debug;

    // ACT & ASSERT
    REQUIRE(game.update(input, debug, dummyDeltaTime) == controller::StateTransitionAction::PushPauseMenu);
}

TEST_CASE("Game update returns PushProgressionStore on mouseLeftPressed")
{
    // ARRANGE
    game::Game game;

    controller::InputState input;
    input.mouseLeftPressed = true;
    controller::DebugState debug;

    // ACT & ASSERT
    REQUIRE(game.update(input, debug, dummyDeltaTime) == controller::StateTransitionAction::PushProgressionStore);
}

TEST_CASE("Game update returns ReplaceCurrentWithGameOverMenu on confirmPressed")
{
    // ARRANGE
    game::Game game;

    controller::InputState input;
    input.confirmPressed = true;
    controller::DebugState debug;

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

TEST_CASE("Game update sets GameDebugState registry when debug is active")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugState debug;
    debug.active = true;

    // PRECONDITION
    REQUIRE(debug.game.registry == nullptr);

    // ACT
    controller::StateTransitionAction action = game.update(input, debug, dummyDeltaTime);

    // ASSERT
    REQUIRE(action == controller::StateTransitionAction::None);
    REQUIRE(debug.game.registry != nullptr);
}

TEST_CASE("Game update does not set GameDebugState registry when debug is inactive")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugState debug;
    debug.active = false;

    // ACT
    controller::StateTransitionAction action = game.update(input, debug, dummyDeltaTime);

    // ASSERT
    REQUIRE(action == controller::StateTransitionAction::None);
    REQUIRE(debug.game.registry == nullptr);
}

TEST_CASE("Game update resets GameDebugState registry and selected entity on game over when debug is active")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    input.confirmPressed = true;

    controller::DebugState debug;
    debug.active = true;
    debug.game.selectedEntity = game::Entity{42};

    // ACT
    controller::StateTransitionAction action = game.update(input, debug, dummyDeltaTime);

    // ASSERT
    REQUIRE(action == controller::StateTransitionAction::ReplaceCurrentWithGameOverMenu);
    REQUIRE(debug.game.registry == nullptr);
    REQUIRE_FALSE(debug.game.selectedEntity.has_value());
}

TEST_CASE("Game update resets GameDebugState stage wave reload request when debug is active")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugState debug;
    debug.active = true;
    debug.game.isStageWaveReloadRequested = true;
    debug.game.stage = 2;
    debug.game.wave = 3;

    // ACT
    controller::StateTransitionAction action = game.update(input, debug, dummyDeltaTime);

    // ASSERT
    REQUIRE(action == controller::StateTransitionAction::None);
    REQUIRE(debug.game.registry != nullptr);
    REQUIRE_FALSE(debug.game.isStageWaveReloadRequested);
}

TEST_CASE("Game update keeps GameDebugState stage wave reload request unchanged when debug is inactive")
{
    // ARRANGE
    game::Game game;
    controller::InputState input;
    controller::DebugState debug;
    debug.active = false;
    debug.game.isStageWaveReloadRequested = true;

    // ACT
    controller::StateTransitionAction action = game.update(input, debug, dummyDeltaTime);

    // ASSERT
    REQUIRE(action == controller::StateTransitionAction::None);
    REQUIRE(debug.game.registry == nullptr);
    REQUIRE(debug.game.isStageWaveReloadRequested);
}