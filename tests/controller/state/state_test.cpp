#include "controller/state/state.hpp"
#include "shared/util.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace controller;

TEST_CASE("MenuState::createMainMenu constructs main menu with expected properties")
{
    // ACT
    std::unique_ptr<MenuState> state = MenuState::createMainMenu();

    // ASSERT
    REQUIRE(state != nullptr);
    REQUIRE(state->type == StateType::MainMenu);
    REQUIRE(state->selectedButtonIndex == 0);
}

TEST_CASE("MenuState::createPauseMenu constructs pause menu with expected properties")
{
    // ACT
    std::unique_ptr<MenuState> state = MenuState::createPauseMenu();

    // ASSERT
    REQUIRE(state != nullptr);
    REQUIRE(state->type == StateType::PauseMenu);
    REQUIRE(state->selectedButtonIndex == 0);
}

TEST_CASE("MenuState::createGameOverMenu constructs game over menu with expected properties")
{
    // ACT
    std::unique_ptr<MenuState> state = MenuState::createGameOverMenu();

    // ASSERT
    REQUIRE(state != nullptr);
    REQUIRE(state->type == StateType::GameOverMenu);
    REQUIRE(state->selectedButtonIndex == 0);
}

TEST_CASE("GameplayState::createGameplay constructs gameplay state with expected properties")
{
    // ACT
    std::unique_ptr<GameplayState> state = GameplayState::createGameplay();

    // ASSERT
    REQUIRE(state != nullptr);
    REQUIRE(state->type == StateType::Gameplay);
}

TEST_CASE("ProgressionStoreState::createStore constructs store state with expected properties")
{
    // ACT
    std::unique_ptr<ProgressionStoreState> state = ProgressionStoreState::createStore();

    // ASSERT
    REQUIRE(state != nullptr);
    REQUIRE(state->type == StateType::ProgressionStore);
}

TEST_CASE("Main menu update returns correct actions")
{
    std::unique_ptr<MenuState> state = MenuState::createMainMenu();

    SECTION("confirm triggers ReplaceCurrentWithGameplay")
    {
        // ARRANGE
        InputState input;
        input.confirm = true;

        // ACT & ASSERT
        REQUIRE(state->update(input, dummyDeltaTime) == StateTransitionAction::ReplaceCurrentWithGameplay);
    }

    SECTION("no relevant input returns None")
    {
        // ARRANGE
        InputState input;

        // ACT & ASSERT
        REQUIRE(state->update(input, dummyDeltaTime) == StateTransitionAction::None);
    }
}

TEST_CASE("Pause menu update returns correct actions")
{
    std::unique_ptr<MenuState> state = MenuState::createPauseMenu();

    SECTION("confirm triggers Pop")
    {
        // ARRANGE
        InputState input;
        input.confirm = true;

        // ACT & ASSERT
        REQUIRE(state->update(input, dummyDeltaTime) == StateTransitionAction::Pop);
    }

    SECTION("no relevant input returns None")
    {
        // ARRANGE
        InputState input;

        // ACT & ASSERT
        REQUIRE(state->update(input, dummyDeltaTime) == StateTransitionAction::None);
    }
}

TEST_CASE("Game over menu update returns correct actions")
{
    std::unique_ptr<MenuState> state = MenuState::createGameOverMenu();

    SECTION("confirm triggers ReplaceCurrentWithMainMenu")
    {
        // ARRANGE
        InputState input;
        input.confirm = true;

        // ACT & ASSERT
        REQUIRE(state->update(input, dummyDeltaTime) == StateTransitionAction::ReplaceCurrentWithMainMenu);
    }

    SECTION("no relevant input returns None")
    {
        // ARRANGE
        InputState input;

        // ACT & ASSERT
        REQUIRE(state->update(input, dummyDeltaTime) == StateTransitionAction::None);
    }
}

TEST_CASE("Gameplay state update returns correct actions")
{
    std::unique_ptr<GameplayState> state = GameplayState::createGameplay();

    SECTION("pause triggers PushPauseMenu")
    {
        // ARRANGE
        InputState input;
        input.pause = true;

        // ACT & ASSERT
        REQUIRE(state->update(input, dummyDeltaTime) == StateTransitionAction::PushPauseMenu);
    }

    SECTION("mouseLeft triggers PushProgressionStore")
    {
        // ARRANGE
        InputState input;
        input.mouseLeft = true;

        // ACT & ASSERT
        REQUIRE(state->update(input, dummyDeltaTime) == StateTransitionAction::PushProgressionStore);
    }

    SECTION("confirm triggers ReplaceCurrentWithGameOverMenu")
    {
        // ARRANGE
        InputState input;
        input.confirm = true;

        // ACT & ASSERT
        REQUIRE(state->update(input, dummyDeltaTime) == StateTransitionAction::ReplaceCurrentWithGameOverMenu);
    }

    SECTION("pause has priority over mouseLeft")
    {
        // ARRANGE
        InputState input;
        input.pause = true;
        input.mouseLeft = true;
        input.confirm = true;

        // ACT & ASSERT
        REQUIRE(state->update(input, dummyDeltaTime) == StateTransitionAction::PushPauseMenu);
    }

    SECTION("mouseLeft has priority over confirm")
    {
        // ARRANGE
        InputState input;
        input.mouseLeft = true;
        input.confirm = true;

        // ACT & ASSERT
        REQUIRE(state->update(input, dummyDeltaTime) == StateTransitionAction::PushProgressionStore);
    }

    SECTION("no relevant input returns None")
    {
        // ARRANGE
        InputState input;

        // ACT & ASSERT
        REQUIRE(state->update(input, dummyDeltaTime) == StateTransitionAction::None);
    }
}

TEST_CASE("ProgressionStoreState update returns correct actions")
{
    std::unique_ptr<ProgressionStoreState> state = ProgressionStoreState::createStore();

    SECTION("confirm triggers Pop")
    {
        // ARRANGE
        InputState input;
        input.confirm = true;

        // ACT & ASSERT
        REQUIRE(state->update(input, dummyDeltaTime) == StateTransitionAction::Pop);
    }

    SECTION("no relevant input returns None")
    {
        // ARRANGE
        InputState input;

        // ACT & ASSERT
        REQUIRE(state->update(input, dummyDeltaTime) == StateTransitionAction::None);
    }
}