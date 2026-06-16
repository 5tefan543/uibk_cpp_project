#include "controller/input/input_state.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "controller/state/state_manager.hpp"
#include "shared/test_fixture.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace controller;

TEST_CASE_METHOD(TestFixture, "StateManager can be constructed")
{
    REQUIRE_NOTHROW(StateManager());
}

TEST_CASE_METHOD(TestFixture, "StateManager is empty initially")
{
    StateManager stateManager;
    REQUIRE(stateManager.isEmpty());
}

TEST_CASE_METHOD(TestFixture, "push adds a state and getCurrent returns it")
{
    // ARRANGE
    StateManager stateManager;

    // ACT
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));

    // ASSERT
    REQUIRE_FALSE(stateManager.isEmpty());
    REQUIRE(typeid(stateManager.getCurrent()) == typeid(MenuState));
}

TEST_CASE_METHOD(TestFixture, "push multiple states and getCurrent returns top")
{
    // ARRANGE
    StateManager stateManager;

    // ACT
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));
    stateManager.push(GameplayState::createNewGameplay(game::CharacterType::Melee));

    // ASSERT
    REQUIRE(typeid(stateManager.getCurrent()) == typeid(GameplayState));
}

TEST_CASE_METHOD(TestFixture, "getCurrent on empty state manager throws")
{
    // ARRANGE
    StateManager stateManager;

    // ACT & ASSERT
    REQUIRE_THROWS(stateManager.getCurrent());
}

TEST_CASE_METHOD(TestFixture, "pop removes the top state")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));
    stateManager.push(GameplayState::createNewGameplay(game::CharacterType::Melee));

    // ACT
    stateManager.pop();

    // ASSERT
    REQUIRE(typeid(stateManager.getCurrent()) == typeid(MenuState));
}

TEST_CASE_METHOD(TestFixture, "pop on single element results in empty state manager")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));

    // ACT
    stateManager.pop();

    // ASSERT
    REQUIRE(stateManager.isEmpty());
}

TEST_CASE_METHOD(TestFixture, "pop on empty state manager throws")
{
    // ARRANGE
    StateManager stateManager;

    // ACT & ASSERT
    REQUIRE_THROWS(stateManager.pop());
}

TEST_CASE_METHOD(TestFixture, "clear removes all states")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));
    stateManager.push(MenuState::createMenu(MenuType::PauseMenu));

    // ACT
    stateManager.clear();

    // ASSERT
    REQUIRE(stateManager.isEmpty());
}

TEST_CASE_METHOD(TestFixture, "replaceCurrent replaces the top state")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));
    stateManager.push(MenuState::createMenu(MenuType::PauseMenu));

    // ACT
    stateManager.replaceCurrent(GameplayState::createNewGameplay(game::CharacterType::Melee));

    // ASSERT
    REQUIRE(typeid(stateManager.getCurrent()) == typeid(GameplayState));
    stateManager.pop();
    REQUIRE(typeid(stateManager.getCurrent()) == typeid(MenuState));
    stateManager.pop();
    REQUIRE(stateManager.isEmpty());
}

TEST_CASE_METHOD(TestFixture, "replaceCurrent on empty state manager throws")
{
    // ARRANGE
    StateManager stateManager;

    // ACT & ASSERT
    REQUIRE_THROWS(stateManager.replaceCurrent(MenuState::createMenu(MenuType::MainMenu)));
}

TEST_CASE_METHOD(TestFixture, "applyAction None does not change current state")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));

    // ACT
    stateManager.applyAction(StateTransitionAction::None);

    // ASSERT
    REQUIRE(typeid(stateManager.getCurrent()) == typeid(MenuState));
}

TEST_CASE_METHOD(TestFixture, "applyAction ReplaceCurrentWithGameplay replaces current state with gameplay")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));

    // ACT
    stateManager.applyAction(StateTransitionAction::StartNewGameRanged);

    // ASSERT
    REQUIRE(typeid(stateManager.getCurrent()) == typeid(GameplayState));
    stateManager.pop();
    REQUIRE(stateManager.isEmpty());
}

TEST_CASE_METHOD(TestFixture, "applyAction StartNewGameMelee replaces current state with melee gameplay")
{
    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::CharacterSelection));

    stateManager.applyAction(StateTransitionAction::StartNewGameMelee);

    REQUIRE(typeid(stateManager.getCurrent()) == typeid(GameplayState));
}

TEST_CASE_METHOD(TestFixture, "applyAction StartNewGameRanged replaces current state with ranged gameplay")
{
    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::CharacterSelection));

    stateManager.applyAction(StateTransitionAction::StartNewGameRanged);

    REQUIRE(typeid(stateManager.getCurrent()) == typeid(GameplayState));
}

TEST_CASE_METHOD(TestFixture, "applyAction ReplaceCurrentWithLoadedGameplay creates gameplay loaded from save")
{
    game::PersistedGame game;
    game.wave = 4;
    game.playerStats.currency = 777;
    game.playerStats.moveSpeed = 360.0f;
    game.playerStats.hasDash = true;
    game.playerStats.score = 12345;
    game.playerStats.maxHealth = 500.0f;
    auto isSaved = PersistenceManager::saveGame(game);
    InputState input;
    REQUIRE(isSaved);

    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));

    stateManager.applyAction(StateTransitionAction::ReplaceCurrentWithLoadedGameplay);

    auto *gameplayState = dynamic_cast<GameplayState *>(&stateManager.getCurrent());
    REQUIRE(gameplayState != nullptr);
    REQUIRE(gameplayState->isLoadedFromPersistedGame());

    const game::PersistedGame loaded = gameplayState->game.getPersistedGame();
    gameplayState->game.update(input, 0.1f); // update once to ensure game session is initialized and values are applied
    const int expectedWave = game.wave;
    const int wavesPerStage = PersistenceManager::getConfig().wavesPerStage;
    const int expectedStage = ((expectedWave - 1) / wavesPerStage) + 1;

    REQUIRE(loaded.wave == expectedWave); // wave is advanced to next wave in loadFromPersistedGame
    REQUIRE(gameplayState->game.getDebugSession().stage == expectedStage);
    REQUIRE(loaded.playerStats.currency == 777);
    REQUIRE(loaded.playerStats.moveSpeed == 360.0f);
}

TEST_CASE_METHOD(TestFixture, "applyAction PushPauseMenu pushes cancelPressed menu on top")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(GameplayState::createNewGameplay(game::CharacterType::Melee));

    // ACT
    stateManager.applyAction(StateTransitionAction::PushPauseMenu);

    // ASSERT
    REQUIRE(dynamic_cast<MenuState *>(&stateManager.getCurrent())->type == MenuType::PauseMenu);
}

TEST_CASE_METHOD(TestFixture, "applyAction PushProgressionStore pushes progression store on top")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(GameplayState::createNewGameplay(game::CharacterType::Melee));

    // ACT
    stateManager.applyAction(StateTransitionAction::PushProgressionStore);

    // ASSERT
    REQUIRE(typeid(stateManager.getCurrent()) == typeid(ProgressionStoreState));
}

TEST_CASE_METHOD(TestFixture, "applyAction ReplaceCurrentWithGameOverMenu replaces current state with game over menu")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(GameplayState::createNewGameplay(game::CharacterType::Melee));

    // ACT
    stateManager.applyAction(StateTransitionAction::ReplaceCurrentWithGameOverMenu);

    // ASSERT
    REQUIRE(dynamic_cast<MenuState *>(&stateManager.getCurrent())->type == MenuType::GameOverMenu);
}

TEST_CASE_METHOD(TestFixture, "applyAction Pop removes the top state")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));
    stateManager.push(MenuState::createMenu(MenuType::PauseMenu));

    // ACT
    stateManager.applyAction(StateTransitionAction::Pop);

    // ASSERT
    REQUIRE(dynamic_cast<MenuState *>(&stateManager.getCurrent())->type == MenuType::MainMenu);
}

TEST_CASE_METHOD(TestFixture, "applyAction ReplaceCurrentWithMainMenu replaces current state with main menu")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));

    // ACT
    stateManager.applyAction(StateTransitionAction::ReplaceCurrentWithMainMenu);

    // ASSERT
    REQUIRE(dynamic_cast<MenuState *>(&stateManager.getCurrent())->type == MenuType::MainMenu);
}

TEST_CASE_METHOD(TestFixture, "applyAction ReplaceCurrentWithExitState clears all states and adds exit state")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));
    stateManager.push(MenuState::createMenu(MenuType::PauseMenu));

    // ACT
    stateManager.applyAction(StateTransitionAction::ReplaceAllStatesWithExit);

    // ASSERT
    REQUIRE(typeid(stateManager.getCurrent()) == typeid(ExitState));
    stateManager.pop();
    REQUIRE(stateManager.isEmpty());
}

TEST_CASE_METHOD(TestFixture, "updateAudio on empty StateManager throws")
{
    StateManager stateManager;

    REQUIRE_THROWS(stateManager.updateAudio());
}

TEST_CASE_METHOD(TestFixture, "updateAudio does not throw for menu state when selection did not change")
{
    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));

    REQUIRE_NOTHROW(stateManager.updateAudio());
}

TEST_CASE_METHOD(TestFixture, "updateAudio throws for menu state when selection changed and assets are missing")
{
    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));

    // Navigate so selectedButtonChanged() returns true on the next updateAudio call
    InputState input;
    input.downPressed = true;
    stateManager.getCurrent().update(input, 0.0f);

    // playSound(kMenuHoverSoundPath) propagates uncaught → throws because asset is absent
    REQUIRE_THROWS(stateManager.updateAudio());
}

TEST_CASE_METHOD(TestFixture, "updateAudio does not throw for progression store when selection did not change")
{
    StateManager stateManager;
    stateManager.push(ProgressionStoreState::createStore());

    REQUIRE_NOTHROW(stateManager.updateAudio());
}

TEST_CASE_METHOD(TestFixture, "updateAudio throws for progression store when selection changed and assets are missing")
{
    StateManager stateManager;
    stateManager.push(ProgressionStoreState::createStore());

    InputState input;
    input.downPressed = true;
    stateManager.getCurrent().update(input, 0.0f);

    REQUIRE_THROWS(stateManager.updateAudio());
}

TEST_CASE_METHOD(TestFixture, "updateAudio does not throw for gameplay state when wave did not change")
{
    StateManager stateManager;
    stateManager.push(GameplayState::createNewGameplay(game::CharacterType::Melee));

    REQUIRE_NOTHROW(stateManager.updateAudio());
}

TEST_CASE_METHOD(TestFixture, "updateAudio throws for gameplay state when wave changed and assets are missing")
{
    // GameplayState::currentWave_ starts at 1; loading wave=3 makes hasWaveChanged() true immediately
    game::PersistedGame saved;
    saved.wave = 3;
    REQUIRE(PersistenceManager::saveGame(saved));

    StateManager stateManager;
    stateManager.push(GameplayState::createLoadedGameplay());

    // playSound(kWaveOverSound) propagates uncaught → throws because asset is absent
    REQUIRE_THROWS(stateManager.updateAudio());
}

TEST_CASE_METHOD(TestFixture, "applyAction audio-safe transitions do not throw when assets are unavailable")
{
    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));

    REQUIRE_NOTHROW(stateManager.applyAction(StateTransitionAction::StartNewGameMelee));
    REQUIRE(typeid(stateManager.getCurrent()) == typeid(GameplayState));

    REQUIRE_NOTHROW(stateManager.applyAction(StateTransitionAction::PushPauseMenu));
    REQUIRE(dynamic_cast<MenuState *>(&stateManager.getCurrent())->type == MenuType::PauseMenu);

    REQUIRE_NOTHROW(stateManager.applyAction(StateTransitionAction::Pop));
    REQUIRE(typeid(stateManager.getCurrent()) == typeid(GameplayState));

    REQUIRE_NOTHROW(stateManager.applyAction(StateTransitionAction::ReplaceCurrentWithGameOverMenu));
    REQUIRE(dynamic_cast<MenuState *>(&stateManager.getCurrent())->type == MenuType::GameOverMenu);
}