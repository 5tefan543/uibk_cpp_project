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
}

TEST_CASE("MenuState::createMenu of type PauseMenu constructs cancelPressed menu with expected properties")
{
    // ACT
    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::PauseMenu);

    // ASSERT
    REQUIRE(state != nullptr);
    REQUIRE(state->type == MenuType::PauseMenu);
}

TEST_CASE("MenuState::createMenu of type GameOverMenu constructs game over menu with expected properties")
{
    // ACT
    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::GameOverMenu);

    // ASSERT
    REQUIRE(state != nullptr);
    REQUIRE(state->type == MenuType::GameOverMenu);
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
    auto state = MenuState::createMenu(MenuType::MainMenu);

    SECTION("confirm on initial selection starts gameplay")
    {
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::ReplaceCurrentWithGameplay);
    }

    SECTION("down changes selection so confirm exits")
    {
        REQUIRE(applyInput(state, DOWN) == StateTransitionAction::None);
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::ReplaceAllStatesWithExit);
    }

    SECTION("up from quit selection returns to start game")
    {
        REQUIRE(applyInput(state, DOWN) == StateTransitionAction::None);
        REQUIRE(applyInput(state, UP) == StateTransitionAction::None);
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::ReplaceCurrentWithGameplay);
    }

    SECTION("irrelevant input returns None")
    {
        REQUIRE(applyInput(state, NONE) == StateTransitionAction::None);
    }
}

TEST_CASE("Pause menu update returns correct actions")
{
    auto state = MenuState::createMenu(MenuType::PauseMenu);

    SECTION("confirm on initial resume selection pops pause menu")
    {
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::Pop);
    }

    SECTION("right changes selection so confirm returns None")
    {
        REQUIRE(applyInput(state, RIGHT) == StateTransitionAction::None);
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::ReplaceAllStatesWithExit);
    }

    SECTION("left from exit selection returns to resume")
    {
        REQUIRE(applyInput(state, RIGHT) == StateTransitionAction::None);
        REQUIRE(applyInput(state, LEFT) == StateTransitionAction::None);
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::Pop);
    }

    SECTION("irrelevant input returns None")
    {
        REQUIRE(applyInput(state, NONE) == StateTransitionAction::None);
    }
}

TEST_CASE("Game over menu update returns correct actions")
{
    auto state = MenuState::createMenu(MenuType::GameOverMenu);

    SECTION("confirm on initial selection returns to main menu")
    {
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::ReplaceCurrentWithMainMenu);
    }

    SECTION("right changes selection so confirm exits game")
    {
        REQUIRE(applyInput(state, RIGHT) == StateTransitionAction::None);
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::ReplaceAllStatesWithExit);
    }

    SECTION("left from quit selection returns to main menu option")
    {
        REQUIRE(applyInput(state, RIGHT) == StateTransitionAction::None);
        REQUIRE(applyInput(state, LEFT) == StateTransitionAction::None);
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::ReplaceCurrentWithMainMenu);
    }

    SECTION("irrelevant input returns None")
    {
        REQUIRE(applyInput(state, NONE) == StateTransitionAction::None);
    }
}

TEST_CASE("Gameplay state update returns correct actions")
{
    // ARRANGE
    std::unique_ptr<GameplayState> state = GameplayState::createGameplay();
    InputState input;
    DebugContext debug;

    auto updateOnce = [&]() { return state->update(input, debug, dummyDeltaTime); };

    auto initializeGameSession = [&]() {
        StateTransitionAction firstAction = updateOnce();
        REQUIRE(firstAction == StateTransitionAction::None);
        REQUIRE(debug.gameSession != nullptr);
    };

    SECTION("normal update returns None and sets gameSession")
    {
        // PRECONDITION
        REQUIRE(debug.gameSession == nullptr);

        // ACT
        StateTransitionAction action = updateOnce();

        // ASSERT
        REQUIRE(action == StateTransitionAction::None);
        REQUIRE(debug.gameSession != nullptr);
    }

    SECTION("cancelPressed returns PushPauseMenu")
    {
        // ARRANGE
        input.cancelPressed = true;

        // ACT
        StateTransitionAction action = updateOnce();

        // ASSERT
        REQUIRE(action == StateTransitionAction::PushPauseMenu);
        REQUIRE(debug.gameSession != nullptr);
    }

    SECTION("game over returns ReplaceCurrentWithGameOverMenu and resets gameSession")
    {
        // ARRANGE
        debug.active = true;
        initializeGameSession();
        debug.gameSession->isPlayerDestructionRequested = true;

        // ACT
        StateTransitionAction action = updateOnce();

        // ASSERT
        REQUIRE(action == StateTransitionAction::ReplaceCurrentWithGameOverMenu);
        REQUIRE(debug.gameSession == nullptr);
    }

    SECTION("store open request returns PushProgressionStore when debug is active")
    {
        // ARRANGE
        debug.active = true;
        initializeGameSession();
        debug.gameSession->isStoreOpenRequested = true;

        // ACT
        StateTransitionAction action = updateOnce();

        // ASSERT
        REQUIRE(action == StateTransitionAction::PushProgressionStore);
        REQUIRE_FALSE(debug.gameSession->isStoreOpenRequested);
    }

    SECTION("store open request is ignored when debug is inactive")
    {
        // ARRANGE
        debug.active = false;
        initializeGameSession();
        debug.gameSession->isStoreOpenRequested = true;

        // ACT
        StateTransitionAction action = updateOnce();

        // ASSERT
        REQUIRE(action == StateTransitionAction::None);
        REQUIRE(debug.gameSession->isStoreOpenRequested);
    }
}

TEST_CASE("ProgressionStoreState update returns correct actions")
{
    std::unique_ptr<ProgressionStoreState> state = ProgressionStoreState::createStore();

    SECTION("confirmPressed triggers Pop")
    {
        // ARRANGE
        InputState input;
        input.confirmPressed = true;
        DebugContext debug;

        // ACT & ASSERT
        REQUIRE(state->update(input, debug, dummyDeltaTime) == StateTransitionAction::Pop);
    }

    SECTION("no relevant input returns None")
    {
        // ARRANGE
        InputState input;
        DebugContext debug;

        // ACT & ASSERT
        REQUIRE(state->update(input, debug, dummyDeltaTime) == StateTransitionAction::None);
    }
}

TEST_CASE("MenuState::toString returns expected string")
{
    SECTION("main menu returns MainMenu")
    {
        std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::MainMenu);
        REQUIRE(state->toString() == "MainMenu");
    }

    SECTION("cancelPressed menu returns PauseMenu")
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

        const View &view = state->getView();
        REQUIRE(view.items.size() == 1);

        const Card &card = ViewItemAccessor::as<const Card>(view.items[0]);
        REQUIRE(card.items.size() == 3);

        const Text &title = ViewItemAccessor::as<const Text>(card.items[0]);
        REQUIRE(title.text == "Main Menu");

        const Button &startButton = ViewItemAccessor::as<const Button>(card.items[1]);
        REQUIRE(startButton.text.text == "Start Game");

        const Button &quitButton = ViewItemAccessor::as<const Button>(card.items[2]);
        REQUIRE(quitButton.text.text == "Quit");
    }

    SECTION("pause menu returns expected view")
    {
        std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::PauseMenu);

        const View &view = state->getView();
        REQUIRE(view.items.size() == 1);

        const Card &pauseCard = ViewItemAccessor::as<const Card>(view.items[0]);
        REQUIRE(pauseCard.items.size() == 3);

        const Text &title = ViewItemAccessor::as<const Text>(pauseCard.items[0]);
        REQUIRE(title.text == "Paused");

        const Button &resumeButton = ViewItemAccessor::as<const Button>(pauseCard.items[1]);
        const Button &quitButton = ViewItemAccessor::as<const Button>(pauseCard.items[2]);

        REQUIRE(resumeButton.text.text == "Resume");
        REQUIRE(resumeButton.centerOffsetX == -100);
        REQUIRE(resumeButton.isSelected == true);

        REQUIRE(quitButton.text.text == "Quit");
        REQUIRE(quitButton.centerOffsetX == 100);
        REQUIRE(quitButton.isSelected == false);
    }

    SECTION("game over menu returns expected view")
    {
        std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::GameOverMenu);

        const View &view = state->getView();
        REQUIRE(view.items.size() == 1);

        const Card &gameOverCard = ViewItemAccessor::as<const Card>(view.items[0]);
        REQUIRE(gameOverCard.items.size() == 3);

        const Text &gameOverText = ViewItemAccessor::as<const Text>(gameOverCard.items[0]);
        REQUIRE(gameOverText.text == "Game Over!");
        REQUIRE(gameOverText.centerOffsetY == -100);

        const Button &mainMenuButton = ViewItemAccessor::as<const Button>(gameOverCard.items[1]);
        REQUIRE(mainMenuButton.text.text == "Main Menu");
        REQUIRE(mainMenuButton.centerOffsetX == -100);
        REQUIRE(mainMenuButton.isSelected == true);

        const Button &quitButton = ViewItemAccessor::as<const Button>(gameOverCard.items[2]);
        REQUIRE(quitButton.text.text == "Quit");
        REQUIRE(quitButton.centerOffsetX == 100);
        REQUIRE(quitButton.isSelected == false);
    }
}

TEST_CASE("GameplayState::getView returns expected view")
{
    std::unique_ptr<GameplayState> state = GameplayState::createGameplay();

    const View &view = state->getView();

    REQUIRE(!view.items.empty());
}

TEST_CASE("ProgressionStoreState::getView returns expected view")
{
    std::unique_ptr<ProgressionStoreState> state = ProgressionStoreState::createStore();

    const View &view = state->getView();

    REQUIRE(view.items.empty());
}

TEST_CASE("ExitState::getView returns expected view")
{
    std::unique_ptr<ExitState> state = ExitState::createExitState();

    const View &view = state->getView();

    REQUIRE(view.items.empty());
}

TEST_CASE("ExitState::update returns ReplaceAllStatesWithExit")
{
    std::unique_ptr<ExitState> state = ExitState::createExitState();

    InputState input;
    DebugContext debug;

    StateTransitionAction action = state->update(input, debug, dummyDeltaTime);

    REQUIRE(action == StateTransitionAction::ReplaceAllStatesWithExit);
}