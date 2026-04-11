#include "controller/state/state.hpp"
#include "shared/util.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace controller;

TEST_CASE("MenuState::createMenu of type MainMenu constructs main menu with expected properties")
{
    // ACT
    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::MainMenu);

    // ASSERT
    REQUIRE(state != nullptr);
    REQUIRE(state->type == MenuType::MainMenu);
    REQUIRE(state->selectedButtonIndex == 0);
}

TEST_CASE("MenuState::createMenu of type PauseMenu constructs pause menu with expected properties")
{
    // ACT
    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::PauseMenu);

    // ASSERT
    REQUIRE(state != nullptr);
    REQUIRE(state->type == MenuType::PauseMenu);
    REQUIRE(state->selectedButtonIndex == 0);
}

TEST_CASE("MenuState::createMenu of type GameOverMenu constructs game over menu with expected properties")
{
    // ACT
    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::GameOverMenu);

    // ASSERT
    REQUIRE(state != nullptr);
    REQUIRE(state->type == MenuType::GameOverMenu);
    REQUIRE(state->selectedButtonIndex == 0);
}

TEST_CASE("GameplayState::createGameplay constructs gameplay state with expected properties")
{
    // ACT
    std::unique_ptr<GameplayState> state = GameplayState::createGameplay();

    // ASSERT
    REQUIRE(state != nullptr);
}

TEST_CASE("ProgressionStoreState::createStore constructs store state with expected properties")
{
    // ACT
    std::unique_ptr<ProgressionStoreState> state = ProgressionStoreState::createStore();

    // ASSERT
    REQUIRE(state != nullptr);
}

TEST_CASE("Main menu update returns correct actions")
{
    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::MainMenu);

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
    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::PauseMenu);

    SECTION("confirm triggers Pop when first button is selected")
    {
        // ARRANGE
        state->selectedButtonIndex = 0;
        InputState input;
        input.confirm = true;

        // ACT & ASSERT
        REQUIRE(state->update(input, dummyDeltaTime) == StateTransitionAction::Pop);
    }

    SECTION("left selects first button")
    {
        // ARRANGE
        state->selectedButtonIndex = 1;
        InputState input;
        input.left = true;

        // ACT
        StateTransitionAction action = state->update(input, dummyDeltaTime);

        // ASSERT
        REQUIRE(action == StateTransitionAction::None);
        REQUIRE(state->selectedButtonIndex == 0);
    }

    SECTION("right selects second button")
    {
        // ARRANGE
        state->selectedButtonIndex = 0;
        InputState input;
        input.right = true;

        // ACT
        StateTransitionAction action = state->update(input, dummyDeltaTime);

        // ASSERT
        REQUIRE(action == StateTransitionAction::None);
        REQUIRE(state->selectedButtonIndex == 1);
    }

    SECTION("confirm on second button does not pop")
    {
        // ARRANGE
        state->selectedButtonIndex = 1;
        InputState input;
        input.confirm = true;

        // ACT & ASSERT
        REQUIRE(state->update(input, dummyDeltaTime) == StateTransitionAction::None);
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
    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::GameOverMenu);

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

TEST_CASE("MenuState::toString returns expected string")
{
    SECTION("main menu returns MainMenu")
    {
        std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::MainMenu);
        REQUIRE(state->toString() == "MainMenu");
    }

    SECTION("pause menu returns PauseMenu")
    {
        std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::PauseMenu);
        REQUIRE(state->toString() == "PauseMenu");
    }

    SECTION("game over menu returns GameOverMenu")
    {
        std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::GameOverMenu);
        REQUIRE(state->toString() == "GameOverMenu");
    }
}

TEST_CASE("GameplayState::toString returns expected string")
{
    std::unique_ptr<GameplayState> state = GameplayState::createGameplay();
    REQUIRE(state->toString() == "Gameplay");
}

TEST_CASE("ProgressionStoreState::toString returns expected string")
{
    std::unique_ptr<ProgressionStoreState> state = ProgressionStoreState::createStore();
    REQUIRE(state->toString() == "ProgressionStore");
}

TEST_CASE("MenuState::getView returns expected view")
{
    SECTION("main menu returns expected view")
    {
        std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::MainMenu);

        View view = state->getView();

        REQUIRE(view.items.size() == 1);
        auto &card = std::get<std::unique_ptr<Card>>(view.items[0]);
        Text title = std::get<Text>(card->items[0]);
        REQUIRE(title.text == "Main Menu");

        Button startButton = std::get<Button>(card->items[1]);
        REQUIRE(startButton.text.text == "Start Game");

        Button quitButton = std::get<Button>(card->items[2]);
        REQUIRE(quitButton.text.text == "Quit");
    }

    SECTION("pause menu returns expected view")
    {
        std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::PauseMenu);
        state->selectedButtonIndex = 0;

        View view = state->getView();

        REQUIRE(view.items.size() == 1);

        auto &pauseCard = std::get<std::unique_ptr<Card>>(view.items[0]);

        REQUIRE(pauseCard->items.size() == 2);

        Button resumeButton = std::get<Button>(pauseCard->items[0]);
        Button quitButton = std::get<Button>(pauseCard->items[1]);

        REQUIRE(resumeButton.text.text == "Resume");
        REQUIRE(resumeButton.centerOffsetX == -100);
        REQUIRE(resumeButton.isSelected == true);

        REQUIRE(quitButton.text.text == "Quit");
        REQUIRE(quitButton.centerOffsetX == 100);
        REQUIRE(quitButton.isSelected == false);
    }

    SECTION("pause menu marks quit button as selected when selectedButtonIndex is 1")
    {
        std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::PauseMenu);
        state->selectedButtonIndex = 1;

        View view = state->getView();

        REQUIRE(view.items.size() == 1);

        auto &pauseCard = std::get<std::unique_ptr<Card>>(view.items[0]);

        REQUIRE(pauseCard->items.size() == 2);

        Button resumeButton = std::get<Button>(pauseCard->items[0]);
        Button quitButton = std::get<Button>(pauseCard->items[1]);

        REQUIRE(resumeButton.isSelected == false);
        REQUIRE(quitButton.isSelected == true);
    }

    SECTION("game over menu returns expected view")
    {
        std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::GameOverMenu);

        View view = state->getView();

        REQUIRE(view.items.empty());
    }
}

TEST_CASE("GameplayState::getView returns expected view")
{
    std::unique_ptr<GameplayState> state = GameplayState::createGameplay();

    View view = state->getView();

    REQUIRE(view.items.size() == 1);
    REQUIRE(std::holds_alternative<controller::Text>(view.items.back()));
}

TEST_CASE("ProgressionStoreState::getView returns expected view")
{
    std::unique_ptr<ProgressionStoreState> state = ProgressionStoreState::createStore();

    View view = state->getView();

    REQUIRE(view.items.empty());
}