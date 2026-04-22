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
    // check that if the button is confirmed, it triggers the expected action
    InputState input;
    input.confirmPressed = true;
    DebugContext debug;
    REQUIRE(state->update(input, debug, dummyDeltaTime) == StateTransitionAction::ReplaceCurrentWithMainMenu);
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
    GIVEN("Main Menu in initial state")
    {
        std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::MainMenu);
        {
            INFO("confirm pressed with initial button selection triggers ReplaceCurrentWithGameplay");
            REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::ReplaceCurrentWithGameplay);
        }
        {
            INFO("down input returns no state transition action");
            REQUIRE(applyInput(state, DOWN) == StateTransitionAction::None);
        }
        {
            INFO("confirm exits game when quit button is selected (through previous down input)");
            REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::ReplaceAllStatesWithExit);
        }
        {
            INFO("no relevant input returns None");
            REQUIRE(applyInput(state, NONE) == StateTransitionAction::None);
        }
        {
            INFO("up input returns no state transition action");
            REQUIRE(applyInput(state, UP) == StateTransitionAction::None);
        }
        {
            INFO("confirmPressed triggers ReplaceCurrentWithGameplay (through previous up input)");
            REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::ReplaceCurrentWithGameplay);
        }
    }
}

TEST_CASE("Pause menu update returns correct actions")
{
    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::PauseMenu);
    {
        INFO("confirmPressed triggers Pop when initial (resume) button is selected");
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::Pop);
    }
    {
        INFO("right input returns no state transition");
        REQUIRE(applyInput(state, RIGHT) == StateTransitionAction::None);
    }
    {
        INFO("confirmPressed on second (exit) button (selected through previous input) does not pop");
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::None);
    }
    {
        INFO("left input returns no state transition");
        REQUIRE(applyInput(state, LEFT) == StateTransitionAction::None);
    }
    {
        INFO("confirmPressed triggers Pop when left (resume) button is selected (through previous input)");
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::Pop);
    }
    {
        INFO("no relevant input returns None");
        REQUIRE(applyInput(state, NONE) == StateTransitionAction::None);
    }
}

TEST_CASE("Game over menu update returns correct actions")
{
    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::GameOverMenu);
    {
        INFO("confirmPressed triggers ReplaceCurrentWithMainMenu");
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::ReplaceCurrentWithMainMenu);
    }
    {
        INFO("right input returns no state transition");
        REQUIRE(applyInput(state, RIGHT) == StateTransitionAction::None);
    }
    {
        INFO("confirm exits game when quit button is selected (through previous input)");
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::ReplaceAllStatesWithExit);
    }
    {
        INFO("left input returns no state transition");
        REQUIRE(applyInput(state, LEFT) == StateTransitionAction::None);
    }
    {
        INFO("confirmPressed triggers ReplaceCurrentWithMainMenu");
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::ReplaceCurrentWithMainMenu);
    }
    {
        INFO("no relevant input returns None");
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
        auto &card = std::get<std::unique_ptr<Card>>(view.items[0]);
        Text title = std::get<Text>(card->items[0]);
        REQUIRE(title.text == "Main Menu");

        std::shared_ptr<Button> startButton = std::get<std::shared_ptr<Button>>(card->items[1]);
        REQUIRE(startButton->text.text == "Start Game");

        std::shared_ptr<Button> quitButton = std::get<std::shared_ptr<Button>>(card->items[2]);
        REQUIRE(quitButton->text.text == "Quit");
    }

    SECTION("cancelPressed menu returns expected view")
    {
        std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::PauseMenu);

        const View &view = state->getView();

        REQUIRE(view.items.size() == 1);

        auto &pauseCard = std::get<std::unique_ptr<Card>>(view.items[0]);

        REQUIRE(pauseCard->items.size() == 2);

        std::shared_ptr<Button> resumeButton = std::get<std::shared_ptr<Button>>(pauseCard->items[0]);
        std::shared_ptr<Button> quitButton = std::get<std::shared_ptr<Button>>(pauseCard->items[1]);

        REQUIRE(resumeButton->text.text == "Resume");
        REQUIRE(resumeButton->centerOffsetX == -100);
        REQUIRE(resumeButton->isSelected == true);

        REQUIRE(quitButton->text.text == "Quit");
        REQUIRE(quitButton->centerOffsetX == 100);
        REQUIRE(quitButton->isSelected == false);
    }

    SECTION("cancelPressed menu marks quit button as selected when selectedButtonIndex is 1")
    {
        std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::PauseMenu);

        const View &view = state->getView();

        REQUIRE(view.items.size() == 1);

        auto &pauseCard = std::get<std::unique_ptr<Card>>(view.items[0]);

        REQUIRE(pauseCard->items.size() == 2);

        std::shared_ptr<Button> resumeButton = std::get<std::shared_ptr<Button>>(pauseCard->items[0]);
        std::shared_ptr<Button> quitButton = std::get<std::shared_ptr<Button>>(pauseCard->items[1]);

        REQUIRE(resumeButton->isSelected == true);
        REQUIRE(quitButton->isSelected == false);
    }

    SECTION("game over menu returns expected view")
    {
        std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::GameOverMenu);

        const View &view = state->getView();
        REQUIRE(view.items.size() == 1);

        auto &gameOverCard = std::get<std::unique_ptr<Card>>(view.items[0]);
        REQUIRE(gameOverCard->items.size() == 3);

        Text gameOverText = std::get<Text>(gameOverCard->items[0]);
        REQUIRE(gameOverText.text == "Game Over!");
        REQUIRE(gameOverText.centerOffsetY == -100);

        std::shared_ptr<Button> mainMenuButton = std::get<std::shared_ptr<Button>>(gameOverCard->items[1]);
        REQUIRE(mainMenuButton->text.text == "Main Menu");
        REQUIRE(mainMenuButton->centerOffsetX == -100);
        REQUIRE(mainMenuButton->isSelected == true);

        std::shared_ptr<Button> quitButton = std::get<std::shared_ptr<Button>>(gameOverCard->items[2]);
        REQUIRE(quitButton->text.text == "Quit");
        REQUIRE(quitButton->centerOffsetX == 100);
        REQUIRE(quitButton->isSelected == false);
    }
}

TEST_CASE("GameplayState::getView returns expected view")
{
    std::unique_ptr<GameplayState> state = GameplayState::createGameplay();

    const View &view = state->getView();

    REQUIRE(view.items.empty());
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