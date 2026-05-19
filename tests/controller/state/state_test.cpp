#include "controller/debug/debug_context.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "controller/state/state.hpp"
#include "shared/test_fixture.hpp"
#include "shared/util.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace controller;

namespace {

void createSavedGameFile()
{
    PersistedGame game;
    game.wave = 2;
    game.currency = 150;
    game.playerStats.speed = 444.0f;
    game.playerStats.hasDash = false;
    game.playerStats.attackPower = 55.0f;
    game.playerStats.attackSpeed = 1.5f;
    game.playerStats.defense = 20.0f;
    PersistenceManager::saveGame(game);
}

} // namespace

TEST_CASE_METHOD(TestFixture, "MenuState::createMenu of type MainMenu constructs main menu with expected properties")
{
    // ACT
    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::MainMenu);

    // ASSERT
    REQUIRE(state != nullptr);
    REQUIRE(state->type == MenuType::MainMenu);
}

TEST_CASE_METHOD(TestFixture,
                 "MenuState::createMenu of type PauseMenu constructs cancelPressed menu with expected properties")
{
    // ACT
    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::PauseMenu);

    // ASSERT
    REQUIRE(state != nullptr);
    REQUIRE(state->type == MenuType::PauseMenu);
}

TEST_CASE_METHOD(TestFixture,
                 "MenuState::createMenu of type GameOverMenu constructs game over menu with expected properties")
{
    // ACT
    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::GameOverMenu);

    // ASSERT
    REQUIRE(state != nullptr);
    REQUIRE(state->type == MenuType::GameOverMenu);
}

TEST_CASE_METHOD(TestFixture, "GameplayState::createNewGameplay constructs gameplay state with expected properties")
{
    // ACT
    std::unique_ptr<GameplayState> state = GameplayState::createNewGameplay();

    // ASSERT
    REQUIRE(state != nullptr);
}

TEST_CASE_METHOD(TestFixture, "ProgressionStoreState::createStore constructs store state with expected properties")
{
    // ACT
    std::unique_ptr<ProgressionStoreState> state = ProgressionStoreState::createStore();

    // ASSERT
    REQUIRE(state != nullptr);
}

TEST_CASE_METHOD(TestFixture, "Main menu update returns correct actions")
{
    auto state = MenuState::createMenu(MenuType::MainMenu);

    SECTION("confirm on initial selection starts gameplay")
    {
        REQUIRE(applyInput<controller::MenuState>(state, CONFIRM) == StateTransitionAction::ReplaceCurrentWithGameplay);
    }

    SECTION("down changes selection so confirm exits")
    {
        REQUIRE(applyInput<controller::MenuState>(state, DOWN) == StateTransitionAction::None);
        REQUIRE(applyInput<controller::MenuState>(state, CONFIRM) == StateTransitionAction::ReplaceAllStatesWithExit);
    }

    SECTION("up from quit selection returns to start game")
    {
        REQUIRE(applyInput<controller::MenuState>(state, DOWN) == StateTransitionAction::None);
        REQUIRE(applyInput<controller::MenuState>(state, UP) == StateTransitionAction::None);
        REQUIRE(applyInput<controller::MenuState>(state, CONFIRM) == StateTransitionAction::ReplaceCurrentWithGameplay);
    }

    SECTION("irrelevant input returns None")
    {
        REQUIRE(applyInput<controller::MenuState>(state, NONE) == StateTransitionAction::None);
    }
}

TEST_CASE_METHOD(TestFixture, "Main menu mouse input returns correct actions")
{
    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::MainMenu);
    const view::View &view = state->getView();
    const view::Card &backgroundCard = ViewElementAccessor::as<const view::Card>(view.nodes[0].element);
    const view::Card &card = ViewElementAccessor::as<const view::Card>(backgroundCard.elements[0]);
    const view::Button &startButton = ViewElementAccessor::as<const view::Button>(card.elements[1]);
    const view::Button &quitButton = ViewElementAccessor::as<const view::Button>(card.elements[2]);

    SECTION("mouse move over quit selects quit without triggering action")
    {
        REQUIRE(applyMouseMove(state, getCenterX(quitButton), getCenterY(quitButton)) == StateTransitionAction::None);

        REQUIRE(startButton.isSelected == false);
        REQUIRE(quitButton.isSelected == true);
    }

    SECTION("mouse click on start game starts gameplay")
    {
        REQUIRE(applyMouseClick<controller::MenuState>(state, getCenterX(startButton), getCenterY(startButton))
                == StateTransitionAction::ReplaceCurrentWithGameplay);
    }

    SECTION("mouse click on quit exits game")
    {
        REQUIRE(applyMouseClick<controller::MenuState>(state, getCenterX(quitButton), getCenterY(quitButton))
                == StateTransitionAction::ReplaceAllStatesWithExit);
    }

    SECTION("mouse click outside buttons returns None")
    {
        REQUIRE(applyMouseClick<controller::MenuState>(state, 700.0f, 400.0f) == StateTransitionAction::None);
    }
}

TEST_CASE_METHOD(TestFixture, "Main menu exposes load option and action when saved game exists")
{
    createSavedGameFile();

    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::MainMenu);

    SECTION("down selects load game and confirm loads gameplay")
    {
        REQUIRE(applyInput<controller::MenuState>(state, DOWN) == StateTransitionAction::None);
        REQUIRE(applyInput<controller::MenuState>(state, CONFIRM)
                == StateTransitionAction::ReplaceCurrentWithLoadedGameplay);
    }

    SECTION("second down selects quit and confirm exits")
    {
        REQUIRE(applyInput<controller::MenuState>(state, DOWN) == StateTransitionAction::None);
        REQUIRE(applyInput<controller::MenuState>(state, DOWN) == StateTransitionAction::None);
        REQUIRE(applyInput<controller::MenuState>(state, CONFIRM) == StateTransitionAction::ReplaceAllStatesWithExit);
    }
}

TEST_CASE_METHOD(TestFixture, "Pause menu update returns correct actions")
{
    auto state = MenuState::createMenu(MenuType::PauseMenu);

    SECTION("confirm on initial resume selection pops pause menu")
    {
        REQUIRE(applyInput<controller::MenuState>(state, CONFIRM) == StateTransitionAction::Pop);
    }

    SECTION("down changes selection so confirm returns None")
    {
        REQUIRE(applyInput<controller::MenuState>(state, DOWN) == StateTransitionAction::None);
        REQUIRE(applyInput<controller::MenuState>(state, CONFIRM) == StateTransitionAction::ReplaceAllStatesWithExit);
    }

    SECTION("up from exit selection returns to resume")
    {
        REQUIRE(applyInput<controller::MenuState>(state, DOWN) == StateTransitionAction::None);
        REQUIRE(applyInput<controller::MenuState>(state, UP) == StateTransitionAction::None);
        REQUIRE(applyInput<controller::MenuState>(state, CONFIRM) == StateTransitionAction::Pop);
    }

    SECTION("irrelevant input returns None")
    {
        REQUIRE(applyInput<controller::MenuState>(state, NONE) == StateTransitionAction::None);
    }
}

TEST_CASE_METHOD(TestFixture, "Pause menu mouse input returns correct actions")
{
    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::PauseMenu);
    const view::View &view = state->getView();
    const view::Card &backgroundCard = ViewElementAccessor::as<const view::Card>(view.nodes[0].element);
    const view::Card &card = ViewElementAccessor::as<const view::Card>(backgroundCard.elements[0]);
    const view::Button &resumeButton = ViewElementAccessor::as<const view::Button>(card.elements[1]);
    const view::Button &quitButton = ViewElementAccessor::as<const view::Button>(card.elements[2]);

    SECTION("mouse move over quit selects quit without triggering action")
    {
        REQUIRE(applyMouseMove(state, getCenterX(quitButton), getCenterY(quitButton)) == StateTransitionAction::None);

        REQUIRE(resumeButton.isSelected == false);
        REQUIRE(quitButton.isSelected == true);
    }

    SECTION("mouse click on resume pops pause menu")
    {
        REQUIRE(applyMouseClick<controller::MenuState>(state, getCenterX(resumeButton), getCenterY(resumeButton))
                == StateTransitionAction::Pop);
    }

    SECTION("mouse click on quit exits game")
    {
        REQUIRE(applyMouseClick<controller::MenuState>(state, getCenterX(quitButton), getCenterY(quitButton))
                == StateTransitionAction::ReplaceAllStatesWithExit);
    }

    SECTION("mouse click outside buttons returns None")
    {
        REQUIRE(applyMouseClick<controller::MenuState>(state, 700.0f, 400.0f) == StateTransitionAction::None);
    }
}

TEST_CASE_METHOD(TestFixture, "Game over menu update returns correct actions")
{
    auto state = MenuState::createMenu(MenuType::GameOverMenu);

    SECTION("confirm on initial selection returns to main menu")
    {
        REQUIRE(applyInput<controller::MenuState>(state, CONFIRM) == StateTransitionAction::ReplaceCurrentWithMainMenu);
    }

    SECTION("down changes selection so confirm exits game")
    {
        REQUIRE(applyInput<controller::MenuState>(state, DOWN) == StateTransitionAction::None);
        REQUIRE(applyInput<controller::MenuState>(state, CONFIRM) == StateTransitionAction::ReplaceAllStatesWithExit);
    }

    SECTION("up from quit selection returns to main menu option")
    {
        REQUIRE(applyInput<controller::MenuState>(state, DOWN) == StateTransitionAction::None);
        REQUIRE(applyInput<controller::MenuState>(state, UP) == StateTransitionAction::None);
        REQUIRE(applyInput<controller::MenuState>(state, CONFIRM) == StateTransitionAction::ReplaceCurrentWithMainMenu);
    }

    SECTION("irrelevant input returns None")
    {
        REQUIRE(applyInput<controller::MenuState>(state, NONE) == StateTransitionAction::None);
    }
}

TEST_CASE_METHOD(TestFixture, "Game over menu mouse input returns correct actions")
{
    std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::GameOverMenu);
    const view::View &view = state->getView();
    const view::Card &backgroundCard = ViewElementAccessor::as<const view::Card>(view.nodes[0].element);
    const view::Card &card = ViewElementAccessor::as<const view::Card>(backgroundCard.elements[0]);
    const view::Button &mainMenuButton = ViewElementAccessor::as<const view::Button>(card.elements[1]);
    const view::Button &quitButton = ViewElementAccessor::as<const view::Button>(card.elements[2]);

    SECTION("mouse move over quit selects quit without triggering action")
    {
        REQUIRE(applyMouseMove(state, getCenterX(quitButton), getCenterY(quitButton)) == StateTransitionAction::None);

        REQUIRE(mainMenuButton.isSelected == false);
        REQUIRE(quitButton.isSelected == true);
    }

    SECTION("mouse click on main menu returns to main menu")
    {
        REQUIRE(applyMouseClick<controller::MenuState>(state, getCenterX(mainMenuButton), getCenterY(mainMenuButton))
                == StateTransitionAction::ReplaceCurrentWithMainMenu);
    }

    SECTION("mouse click on quit exits game")
    {
        REQUIRE(applyMouseClick<controller::MenuState>(state, getCenterX(quitButton), getCenterY(quitButton))
                == StateTransitionAction::ReplaceAllStatesWithExit);
    }
}

TEST_CASE_METHOD(TestFixture, "Gameplay state update returns correct actions")
{
    // ARRANGE
    std::unique_ptr<GameplayState> state = GameplayState::createNewGameplay();
    InputState input;
    DebugContext &debug = DebugContext::get();

    auto updateOnce = [&]() { return state->update(input, dummyDeltaTime); };

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

    SECTION("player destruction request returns ReplaceCurrentWithGameOverMenu and resets gameSession")
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

    SECTION("game over in debug mode deletes save and transitions to game over menu")
    {
        // ARRANGE
        debug.active = true;
        initializeGameSession();

        // create dummy save to verify deletion
        PersistenceManager::saveGame(PersistedGame{});
        REQUIRE(PersistenceManager::hasSavedGame());

        // Set player destruction request to trigger game over condition
        debug.gameSession->isPlayerDestructionRequested = true;

        // ACT
        StateTransitionAction action = updateOnce();

        // ASSERT
        REQUIRE(action == StateTransitionAction::ReplaceCurrentWithGameOverMenu);
        REQUIRE(debug.gameSession == nullptr);
        REQUIRE_FALSE(PersistenceManager::hasSavedGame());
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

TEST_CASE_METHOD(TestFixture, "ProgressionStoreState update returns correct actions")
{
    std::unique_ptr<ProgressionStoreState> state = ProgressionStoreState::createStore();

    SECTION("confirmPressed triggers Pop")
    {
        // ARRANGE
        InputState input;
        input.confirmPressed = true;

        // ACT & ASSERT
        REQUIRE(state->update(input, dummyDeltaTime) == StateTransitionAction::Pop);
    }

    SECTION("no relevant input returns None")
    {
        // ARRANGE
        InputState input;

        // ACT & ASSERT
        REQUIRE(applyInput<controller::ProgressionStoreState>(state, NONE) == StateTransitionAction::None);
    }

    SECTION("down selects quit and confirm exits")
    {
        REQUIRE(applyInput<controller::ProgressionStoreState>(state, DOWN) == StateTransitionAction::None);
        REQUIRE(applyInput<controller::ProgressionStoreState>(state, CONFIRM)
                == StateTransitionAction::ReplaceAllStatesWithExit);
    }

    SECTION("up from initial selection wraps to quit and confirm exits")
    {
        REQUIRE(applyInput<controller::ProgressionStoreState>(state, UP) == StateTransitionAction::None);
        REQUIRE(applyInput<controller::ProgressionStoreState>(state, CONFIRM)
                == StateTransitionAction::ReplaceAllStatesWithExit);
    }

    const view::View &view = state->getView();
    const view::Card &backgroundCard = ViewElementAccessor::as<const view::Card>(view.nodes[0].element);
    const view::Card &card = ViewElementAccessor::as<const view::Card>(backgroundCard.elements[0]);
    const view::Button &continueButton = ViewElementAccessor::as<const view::Button>(card.elements[1]);
    const view::Button &quitButton = ViewElementAccessor::as<const view::Button>(card.elements[2]);

    SECTION("mouse click on quit exits")
    {

        REQUIRE(
            applyMouseClick<controller::ProgressionStoreState>(state, getCenterX(quitButton), getCenterY(quitButton))
            == StateTransitionAction::ReplaceAllStatesWithExit);
    }

    SECTION("mouse click on continue pops store state")
    {

        REQUIRE(applyMouseClick<controller::ProgressionStoreState>(state, getCenterX(continueButton),
                                                                   getCenterY(continueButton))
                == StateTransitionAction::Pop);
    }
}

TEST_CASE_METHOD(TestFixture, "MenuState::toString returns expected string")
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

TEST_CASE_METHOD(TestFixture, "GameplayState::toString returns expected string")
{
    std::unique_ptr<GameplayState> state = GameplayState::createNewGameplay();
    REQUIRE(state->toString() == "Gameplay");
}

TEST_CASE_METHOD(TestFixture, "ProgressionStoreState::toString returns expected string")
{
    std::unique_ptr<ProgressionStoreState> state = ProgressionStoreState::createStore();
    REQUIRE(state->toString() == "ProgressionStore");
}

TEST_CASE_METHOD(TestFixture, "MenuState::getView returns expected view")
{
    SECTION("main menu returns expected view")
    {
        std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::MainMenu);

        const view::View &view = state->getView();
        REQUIRE(view.nodes.size() == 1);

        const view::Card &backgroundCard = ViewElementAccessor::as<const view::Card>(view.nodes[0].element);
        const view::Card &card = ViewElementAccessor::as<const view::Card>(backgroundCard.elements[0]);
        REQUIRE(card.elements.size() == 3);

        const view::Text &title = ViewElementAccessor::as<const view::Text>(card.elements[0]);
        REQUIRE(title.text == "Main Menu");

        const view::Button &startButton = ViewElementAccessor::as<const view::Button>(card.elements[1]);
        REQUIRE(startButton.text.text == "Start Game");

        const view::Button &quitButton = ViewElementAccessor::as<const view::Button>(card.elements[2]);
        REQUIRE(quitButton.text.text == "Quit");
    }

    SECTION("main menu with save returns expected view including load")
    {
        createSavedGameFile();

        std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::MainMenu);

        const view::View &view = state->getView();
        REQUIRE(view.nodes.size() == 1);
        const view::Card &backgroundCard = ViewElementAccessor::as<const view::Card>(view.nodes[0].element);

        const view::Card &card = ViewElementAccessor::as<const view::Card>(backgroundCard.elements[0]);
        REQUIRE(card.elements.size() == 4);

        const view::Text &title = ViewElementAccessor::as<const view::Text>(card.elements[1]);
        REQUIRE(title.text == "Main Menu");

        const view::Button &startButton = ViewElementAccessor::as<const view::Button>(card.elements[2]);
        REQUIRE(startButton.text.text == "Start Game");

        const view::Button &loadButton = ViewElementAccessor::as<const view::Button>(card.elements[0]);
        REQUIRE(loadButton.text.text == "Load Game");

        const view::Button &quitButton = ViewElementAccessor::as<const view::Button>(card.elements[3]);
        REQUIRE(quitButton.text.text == "Quit");
    }

    SECTION("pause menu returns expected view")
    {
        std::unique_ptr<MenuState> state = MenuState::createMenu(MenuType::PauseMenu);

        const view::View &view = state->getView();
        REQUIRE(view.nodes.size() == 1);

        const view::Card &backgroundCard = ViewElementAccessor::as<const view::Card>(view.nodes[0].element);
        const view::Card &pauseCard = ViewElementAccessor::as<const view::Card>(backgroundCard.elements[0]);
        REQUIRE(pauseCard.elements.size() == 3);

        const view::Text &title = ViewElementAccessor::as<const view::Text>(pauseCard.elements[0]);
        REQUIRE(title.text == "Paused");

        const view::Button &resumeButton = ViewElementAccessor::as<const view::Button>(pauseCard.elements[1]);
        const view::Button &quitButton = ViewElementAccessor::as<const view::Button>(pauseCard.elements[2]);

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
        REQUIRE(view.nodes.size() == 1);

        const view::Card &backgroundCard = ViewElementAccessor::as<const view::Card>(view.nodes[0].element);
        const view::Card &gameOverCard = ViewElementAccessor::as<const view::Card>(backgroundCard.elements[0]);
        REQUIRE(gameOverCard.elements.size() == 3);

        const view::Text &gameOverText = ViewElementAccessor::as<const view::Text>(gameOverCard.elements[0]);
        REQUIRE(gameOverText.text == "Game Over!");
        REQUIRE(gameOverText.gridY == (gameOverCard.gridY + gameOverCard.height / 10));

        const view::Button &mainMenuButton = ViewElementAccessor::as<const view::Button>(gameOverCard.elements[1]);
        REQUIRE(mainMenuButton.text.text == "Main Menu");
        REQUIRE(getCenterY(mainMenuButton) == getCenterY(gameOverCard) - mainMenuButton.height);
        REQUIRE(mainMenuButton.isSelected == true);

        const view::Button &quitButton = ViewElementAccessor::as<const view::Button>(gameOverCard.elements[2]);
        REQUIRE(quitButton.text.text == "Quit");
        REQUIRE(getCenterY(quitButton) == getCenterY(gameOverCard) + mainMenuButton.height);
        REQUIRE(quitButton.isSelected == false);
    }
}

TEST_CASE_METHOD(TestFixture, "GameplayState::getView returns expected view")
{
    std::unique_ptr<GameplayState> state = GameplayState::createNewGameplay();

    const view::View &view = state->getView();

    REQUIRE(!view.nodes.empty());
}

TEST_CASE_METHOD(TestFixture, "ProgressionStoreState::getView returns expected view")
{
    std::unique_ptr<ProgressionStoreState> state = ProgressionStoreState::createStore();

    const view::View &view = state->getView();
    REQUIRE(view.nodes.size() == 1);

    const view::Card &backgroundCard = ViewElementAccessor::as<const view::Card>(view.nodes[0].element);
    const view::Card &storeCard = ViewElementAccessor::as<const view::Card>(backgroundCard.elements[0]);
    REQUIRE(storeCard.elements.size() == 3);

    const view::Text &storeText = ViewElementAccessor::as<const view::Text>(storeCard.elements[0]);
    REQUIRE(storeText.text == "Store Menu");
    REQUIRE(storeText.gridY == (storeCard.gridY + storeCard.height / 10));

    const view::Button &mainMenuButton = ViewElementAccessor::as<const view::Button>(storeCard.elements[1]);
    REQUIRE(mainMenuButton.text.text == "Continue Game");
    REQUIRE(getCenterY(mainMenuButton) == getCenterY(storeCard) - mainMenuButton.height);
    REQUIRE(mainMenuButton.isSelected == true);

    const view::Button &quitButton = ViewElementAccessor::as<const view::Button>(storeCard.elements[2]);
    REQUIRE(quitButton.text.text == "Quit Game");
    REQUIRE(getCenterY(quitButton) == getCenterY(storeCard) + mainMenuButton.height);
    REQUIRE(quitButton.isSelected == false);
}

TEST_CASE_METHOD(TestFixture, "ExitState::getView returns expected view")
{
    std::unique_ptr<ExitState> state = ExitState::createExitState();

    const view::View &view = state->getView();

    REQUIRE(view.nodes.empty());
}

TEST_CASE_METHOD(TestFixture, "ExitState::update returns ReplaceAllStatesWithExit")
{
    std::unique_ptr<ExitState> state = ExitState::createExitState();

    InputState input;

    StateTransitionAction action = state->update(input, dummyDeltaTime);

    REQUIRE(action == StateTransitionAction::ReplaceAllStatesWithExit);
}