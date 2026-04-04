#include "controller/state/state_manager.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace controller;

TEST_CASE("StateManager can be constructed")
{
    REQUIRE_NOTHROW(StateManager());
}

TEST_CASE("StateManager is empty initially")
{
    StateManager stateManager;
    REQUIRE(stateManager.isEmpty());
}

TEST_CASE("push adds a state and getCurrent returns it")
{
    // ARRANGE
    StateManager stateManager;

    // ACT
    stateManager.push(MenuState::createMainMenu());

    // ASSERT
    REQUIRE_FALSE(stateManager.isEmpty());
    REQUIRE(dynamic_cast<MenuState *>(&stateManager.getCurrent())->type == MenuType::MainMenu);
}

TEST_CASE("push multiple states and getCurrent returns top")
{
    // ARRANGE
    StateManager stateManager;

    // ACT
    stateManager.push(MenuState::createMainMenu());
    stateManager.push(MenuState::createPauseMenu());

    // ASSERT
    REQUIRE(dynamic_cast<MenuState *>(&stateManager.getCurrent())->type == MenuType::PauseMenu);
}

TEST_CASE("getCurrent on empty state manager throws")
{
    // ARRANGE
    StateManager stateManager;

    // ACT & ASSERT
    REQUIRE_THROWS(stateManager.getCurrent());
}

TEST_CASE("pop removes the top state")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMainMenu());
    stateManager.push(MenuState::createPauseMenu());

    // ACT
    stateManager.pop();

    // ASSERT
    REQUIRE(dynamic_cast<MenuState *>(&stateManager.getCurrent())->type == MenuType::MainMenu);
}

TEST_CASE("pop on single element results in empty state manager")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMainMenu());

    // ACT
    stateManager.pop();

    // ASSERT
    REQUIRE(stateManager.isEmpty());
}

TEST_CASE("pop on empty state manager throws")
{
    // ARRANGE
    StateManager stateManager;

    // ACT & ASSERT
    REQUIRE_THROWS(stateManager.pop());
}

TEST_CASE("clear removes all states")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMainMenu());
    stateManager.push(MenuState::createPauseMenu());

    // ACT
    stateManager.clear();

    // ASSERT
    REQUIRE(stateManager.isEmpty());
}

TEST_CASE("replaceCurrent replaces the top state")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMainMenu());
    stateManager.push(MenuState::createPauseMenu());

    // ACT
    stateManager.replaceCurrent(MenuState::createGameOverMenu());

    // ASSERT
    REQUIRE(dynamic_cast<MenuState *>(&stateManager.getCurrent())->type == MenuType::GameOverMenu);
}

TEST_CASE("replaceCurrent on empty state manager throws")
{
    // ARRANGE
    StateManager stateManager;

    // ACT & ASSERT
    REQUIRE_THROWS(stateManager.replaceCurrent(MenuState::createMainMenu()));
}

TEST_CASE("applyAction None does not change current state")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMainMenu());

    // ACT
    stateManager.applyAction(StateTransitionAction::None);

    // ASSERT
    REQUIRE(dynamic_cast<MenuState *>(&stateManager.getCurrent())->type == MenuType::MainMenu);
}

TEST_CASE("applyAction ReplaceCurrentWithGameplay replaces current state with gameplay")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMainMenu());

    // ACT
    stateManager.applyAction(StateTransitionAction::ReplaceCurrentWithGameplay);

    // ASSERT
    REQUIRE(dynamic_cast<GameplayState *>(&stateManager.getCurrent()) != nullptr);
}

TEST_CASE("applyAction PushPauseMenu pushes pause menu on top")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(GameplayState::createGameplay());

    // ACT
    stateManager.applyAction(StateTransitionAction::PushPauseMenu);

    // ASSERT
    REQUIRE(dynamic_cast<MenuState *>(&stateManager.getCurrent())->type == MenuType::PauseMenu);
}

TEST_CASE("applyAction PushProgressionStore pushes progression store on top")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(GameplayState::createGameplay());

    // ACT
    stateManager.applyAction(StateTransitionAction::PushProgressionStore);

    // ASSERT
    REQUIRE(dynamic_cast<ProgressionStoreState *>(&stateManager.getCurrent()) != nullptr);
}

TEST_CASE("applyAction ReplaceCurrentWithGameOverMenu replaces current state with game over menu")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(GameplayState::createGameplay());

    // ACT
    stateManager.applyAction(StateTransitionAction::ReplaceCurrentWithGameOverMenu);

    // ASSERT
    REQUIRE(dynamic_cast<MenuState *>(&stateManager.getCurrent())->type == MenuType::GameOverMenu);
}

TEST_CASE("applyAction Pop removes the top state")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMainMenu());
    stateManager.push(MenuState::createPauseMenu());

    // ACT
    stateManager.applyAction(StateTransitionAction::Pop);

    // ASSERT
    REQUIRE(dynamic_cast<MenuState *>(&stateManager.getCurrent())->type == MenuType::MainMenu);
}

TEST_CASE("applyAction ReplaceCurrentWithMainMenu replaces current state with main menu")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMainMenu());

    // ACT
    stateManager.applyAction(StateTransitionAction::ReplaceCurrentWithMainMenu);

    // ASSERT
    REQUIRE(dynamic_cast<MenuState *>(&stateManager.getCurrent())->type == MenuType::MainMenu);
}