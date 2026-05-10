#include "controller/persistence/persistence_manager.hpp"
#include "controller/state/state.hpp"
#include "shared/test_filesystem.hpp"
#include "shared/util.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace controller;

namespace {

void createSavedGameFile()
{
    PersistedGame game;
    game.stage = 3;
    game.wave = 2;
    game.currency = 150;
    game.playerStats.speed = 444.0f;
    PersistenceManager::saveGame(game);
}

} // namespace

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
    test::ScopedTestDirectory testDir("roguelike-state-test-");
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

TEST_CASE("Main menu mouse input returns correct actions")
{
    test::ScopedTestDirectory testDir("roguelike-state-test-");
    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::MainMenu);
    const view::View &view = state->getView();
    const view::Card &backgroundCard = ViewItemAccessor::as<const view::Card>(view.items[0]);
    const view::Card &card = ViewItemAccessor::as<const view::Card>(backgroundCard.items[0]);
    const view::Button &startButton = ViewItemAccessor::as<const view::Button>(card.items[1]);
    const view::Button &quitButton = ViewItemAccessor::as<const view::Button>(card.items[2]);

    SECTION("mouse move over quit selects quit without triggering action")
    {
        REQUIRE(applyMouseMove(state, getCenterX(quitButton), getCenterY(quitButton)) == StateTransitionAction::None);

        REQUIRE(startButton.isSelected == false);
        REQUIRE(quitButton.isSelected == true);
    }

    SECTION("mouse click on start game starts gameplay")
    {
        REQUIRE(applyMouseClick(state, getCenterX(startButton), getCenterY(startButton))
                == StateTransitionAction::ReplaceCurrentWithGameplay);
    }

    SECTION("mouse click on quit exits game")
    {
        REQUIRE(applyMouseClick(state, getCenterX(quitButton), getCenterY(quitButton))
                == StateTransitionAction::ReplaceAllStatesWithExit);
    }

    SECTION("mouse click outside buttons returns None")
    {
        REQUIRE(applyMouseClick(state, 700.0f, 400.0f) == StateTransitionAction::None);
    }
}

TEST_CASE("Main menu exposes load option and action when saved game exists")
{
    test::ScopedTestDirectory testDir("roguelike-state-test-");
    createSavedGameFile();

    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::MainMenu);

    SECTION("down selects load game and confirm loads gameplay")
    {
        REQUIRE(applyInput(state, DOWN) == StateTransitionAction::None);
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::ReplaceCurrentWithLoadedGameplay);
    }

    SECTION("second down selects quit and confirm exits")
    {
        REQUIRE(applyInput(state, DOWN) == StateTransitionAction::None);
        REQUIRE(applyInput(state, DOWN) == StateTransitionAction::None);
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::ReplaceAllStatesWithExit);
    }

    //     SECTION("view contains load game button only when save exists")
    //     {
    //         const view::View &view = state->getView();
    //         const view::Card &card = ViewItemAccessor::as<const view::Card>(view.items[0]);

    // //        REQUIRE(card.items.size() == 1);

    //         const view::Button &startButton = ViewItemAccessor::as<const view::Button>(card.items[1]);
    //         const view::Button &loadButton = ViewItemAccessor::as<const view::Button>(card.items[2]);
    //         const view::Button &quitButton = ViewItemAccessor::as<const view::Button>(card.items[3]);

    //         REQUIRE(startButton.text.text == "Start Game");
    //         REQUIRE(loadButton.text.text == "Load Game");
    //         REQUIRE(quitButton.text.text == "Quit");
    //     }
}

TEST_CASE("Pause menu update returns correct actions")
{
    auto state = MenuState::createMenu(MenuType::PauseMenu);

    SECTION("confirm on initial resume selection pops pause menu")
    {
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::Pop);
    }

    SECTION("down changes selection so confirm returns None")
    {
        REQUIRE(applyInput(state, DOWN) == StateTransitionAction::None);
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::ReplaceAllStatesWithExit);
    }

    SECTION("up from exit selection returns to resume")
    {
        REQUIRE(applyInput(state, DOWN) == StateTransitionAction::None);
        REQUIRE(applyInput(state, UP) == StateTransitionAction::None);
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::Pop);
    }

    SECTION("irrelevant input returns None")
    {
        REQUIRE(applyInput(state, NONE) == StateTransitionAction::None);
    }
}

TEST_CASE("Pause menu mouse input returns correct actions")
{
    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::PauseMenu);
    const view::View &view = state->getView();
    const view::Card &backgroundCard = ViewItemAccessor::as<const view::Card>(view.items[0]);
    const view::Card &card = ViewItemAccessor::as<const view::Card>(backgroundCard.items[0]);
    const view::Button &resumeButton = ViewItemAccessor::as<const view::Button>(card.items[1]);
    const view::Button &quitButton = ViewItemAccessor::as<const view::Button>(card.items[2]);

    SECTION("mouse move over quit selects quit without triggering action")
    {
        REQUIRE(applyMouseMove(state, getCenterX(quitButton), getCenterY(quitButton)) == StateTransitionAction::None);

        REQUIRE(resumeButton.isSelected == false);
        REQUIRE(quitButton.isSelected == true);
    }

    SECTION("mouse click on resume pops pause menu")
    {
        REQUIRE(applyMouseClick(state, getCenterX(resumeButton), getCenterY(resumeButton))
                == StateTransitionAction::Pop);
    }

    SECTION("mouse click on quit exits game")
    {
        REQUIRE(applyMouseClick(state, getCenterX(quitButton), getCenterY(quitButton))
                == StateTransitionAction::ReplaceAllStatesWithExit);
    }

    SECTION("mouse click outside buttons returns None")
    {
        REQUIRE(applyMouseClick(state, 700.0f, 400.0f) == StateTransitionAction::None);
    }
}

TEST_CASE("Game over menu update returns correct actions")
{
    auto state = MenuState::createMenu(MenuType::GameOverMenu);

    SECTION("confirm on initial selection returns to main menu")
    {
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::ReplaceCurrentWithMainMenu);
    }

    SECTION("down changes selection so confirm exits game")
    {
        REQUIRE(applyInput(state, DOWN) == StateTransitionAction::None);
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::ReplaceAllStatesWithExit);
    }

    SECTION("up from quit selection returns to main menu option")
    {
        REQUIRE(applyInput(state, DOWN) == StateTransitionAction::None);
        REQUIRE(applyInput(state, UP) == StateTransitionAction::None);
        REQUIRE(applyInput(state, CONFIRM) == StateTransitionAction::ReplaceCurrentWithMainMenu);
    }

    SECTION("irrelevant input returns None")
    {
        REQUIRE(applyInput(state, NONE) == StateTransitionAction::None);
    }
}

TEST_CASE("Game over menu mouse input returns correct actions")
{
    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::GameOverMenu);
    const view::View &view = state->getView();
    const view::Card &backgroundCard = ViewItemAccessor::as<const view::Card>(view.items[0]);
    const view::Card &card = ViewItemAccessor::as<const view::Card>(backgroundCard.items[0]);
    const view::Button &mainMenuButton = ViewItemAccessor::as<const view::Button>(card.items[1]);
    const view::Button &quitButton = ViewItemAccessor::as<const view::Button>(card.items[2]);

    SECTION("mouse move over quit selects quit without triggering action")
    {
        REQUIRE(applyMouseMove(state, getCenterX(quitButton), getCenterY(quitButton)) == StateTransitionAction::None);

        REQUIRE(mainMenuButton.isSelected == false);
        REQUIRE(quitButton.isSelected == true);
    }

    SECTION("mouse click on main menu returns to main menu")
    {
        REQUIRE(applyMouseClick(state, getCenterX(mainMenuButton), getCenterY(mainMenuButton))
                == StateTransitionAction::ReplaceCurrentWithMainMenu);
    }

    SECTION("mouse click on quit exits game")
    {
        REQUIRE(applyMouseClick(state, getCenterX(quitButton), getCenterY(quitButton))
                == StateTransitionAction::ReplaceAllStatesWithExit);
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
        test::ScopedTestDirectory testDir("roguelike-state-test-");
        std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::MainMenu);

        const view::View &view = state->getView();
        REQUIRE(view.items.size() == 1);

        const view::Card &backgroundCard = ViewItemAccessor::as<const view::Card>(view.items[0]);
        const view::Card &card = ViewItemAccessor::as<const view::Card>(backgroundCard.items[0]);
        REQUIRE(card.items.size() == 3);

        const view::Text &title = ViewItemAccessor::as<const view::Text>(card.items[0]);
        REQUIRE(title.text == "Main Menu");

        const view::Button &startButton = ViewItemAccessor::as<const view::Button>(card.items[1]);
        REQUIRE(startButton.text.text == "Start Game");

        const view::Button &quitButton = ViewItemAccessor::as<const view::Button>(card.items[2]);
        REQUIRE(quitButton.text.text == "Quit");
    }

    //     SECTION("main menu with save returns expected view including load")
    //     {
    //         test::ScopedTestDirectory testDir("roguelike-state-test-");
    //         createSavedGameFile();

    //         std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::MainMenu);

    //         const view::View &view = state->getView();
    //         REQUIRE(view.items.size() == 1);

    //         const view::Card &card = ViewItemAccessor::as<const view::Card>(view.items[0]);
    // //        REQUIRE(card.items.size() == 4);

    //         const view::Text &title = ViewItemAccessor::as<const view::Text>(card.items[0]);
    //         REQUIRE(title.text == "Main Menu");

    //         const view::Button &startButton = ViewItemAccessor::as<const view::Button>(card.items[1]);
    //         REQUIRE(startButton.text.text == "Start Game");

    //         const view::Button &loadButton = ViewItemAccessor::as<const view::Button>(card.items[2]);
    //         REQUIRE(loadButton.text.text == "Load Game");

    //         const view::Button &quitButton = ViewItemAccessor::as<const view::Button>(card.items[3]);
    //         REQUIRE(quitButton.text.text == "Quit");
    //     }

    SECTION("pause menu returns expected view")
    {
        std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::PauseMenu);

        const view::View &view = state->getView();
        REQUIRE(view.items.size() == 1);

        const view::Card &backgroundCard = ViewItemAccessor::as<const view::Card>(view.items[0]);
        const view::Card &pauseCard = ViewItemAccessor::as<const view::Card>(backgroundCard.items[0]);
        REQUIRE(pauseCard.items.size() == 3);

        const view::Text &title = ViewItemAccessor::as<const view::Text>(pauseCard.items[0]);
        REQUIRE(title.text == "Paused");

        const view::Button &resumeButton = ViewItemAccessor::as<const view::Button>(pauseCard.items[1]);
        const view::Button &quitButton = ViewItemAccessor::as<const view::Button>(pauseCard.items[2]);

        REQUIRE(resumeButton.text.text == "Resume");
        REQUIRE(getCenterY(resumeButton) == getCenterY(pauseCard) - resumeButton.height);
        REQUIRE(resumeButton.isSelected == true);

        REQUIRE(quitButton.text.text == "Quit");
        REQUIRE(getCenterY(quitButton) == getCenterY(pauseCard) + quitButton.height);
        REQUIRE(quitButton.isSelected == false);
    }

    SECTION("game over menu returns expected view")
    {
        std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::GameOverMenu);

        const view::View &view = state->getView();
        REQUIRE(view.items.size() == 1);

        const view::Card &backgroundCard = ViewItemAccessor::as<const view::Card>(view.items[0]);
        const view::Card &gameOverCard = ViewItemAccessor::as<const view::Card>(backgroundCard.items[0]);
        REQUIRE(gameOverCard.items.size() == 3);

        const view::Text &gameOverText = ViewItemAccessor::as<const view::Text>(gameOverCard.items[0]);
        REQUIRE(gameOverText.text == "Game Over!");
        REQUIRE(gameOverText.gridY == (gameOverCard.gridY + gameOverCard.height / 10));

        const view::Button &mainMenuButton = ViewItemAccessor::as<const view::Button>(gameOverCard.items[1]);
        REQUIRE(mainMenuButton.text.text == "Main Menu");
        REQUIRE(getCenterY(mainMenuButton) == getCenterY(gameOverCard) - mainMenuButton.height);
        REQUIRE(mainMenuButton.isSelected == true);

        const view::Button &quitButton = ViewItemAccessor::as<const view::Button>(gameOverCard.items[2]);
        REQUIRE(quitButton.text.text == "Quit");
        REQUIRE(getCenterY(quitButton) == getCenterY(gameOverCard) + mainMenuButton.height);
        REQUIRE(quitButton.isSelected == false);
    }
}

TEST_CASE("GameplayState::getView returns expected view")
{
    std::unique_ptr<GameplayState> state = GameplayState::createGameplay();

    const view::View &view = state->getView();

    REQUIRE(!view.items.empty());
}

TEST_CASE("ProgressionStoreState::getView returns expected view")
{
    std::unique_ptr<ProgressionStoreState> state = ProgressionStoreState::createStore();

    const view::View &view = state->getView();

    REQUIRE(view.items.empty());
}

TEST_CASE("ExitState::getView returns expected view")
{
    std::unique_ptr<ExitState> state = ExitState::createExitState();

    const view::View &view = state->getView();

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