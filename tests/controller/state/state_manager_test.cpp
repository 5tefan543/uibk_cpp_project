#include "controller/persistence/persistence_manager.hpp"
#include "controller/state/state_manager.hpp"
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <filesystem>

using namespace controller;

namespace {

namespace fs = std::filesystem;

class ScopedCurrentPath {
  public:
    explicit ScopedCurrentPath(const fs::path &newPath) : oldPath_(fs::current_path())
    {
        fs::create_directories(newPath);
        fs::current_path(newPath);
    }

    ~ScopedCurrentPath() { fs::current_path(oldPath_); }

  private:
    fs::path oldPath_;
};

fs::path createTempTestDirectory()
{
    const auto uniquePart = std::chrono::steady_clock::now().time_since_epoch().count();
    const fs::path dir = fs::temp_directory_path() / ("roguelike-state-manager-test-" + std::to_string(uniquePart));
    fs::create_directories(dir);
    return dir;
}

} // namespace

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
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));

    // ASSERT
    REQUIRE_FALSE(stateManager.isEmpty());
    REQUIRE(typeid(stateManager.getCurrent()) == typeid(MenuState));
}

TEST_CASE("push multiple states and getCurrent returns top")
{
    // ARRANGE
    StateManager stateManager;

    // ACT
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));
    stateManager.push(GameplayState::createGameplay());

    // ASSERT
    REQUIRE(typeid(stateManager.getCurrent()) == typeid(GameplayState));
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
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));
    stateManager.push(GameplayState::createGameplay());

    // ACT
    stateManager.pop();

    // ASSERT
    REQUIRE(typeid(stateManager.getCurrent()) == typeid(MenuState));
}

TEST_CASE("pop on single element results in empty state manager")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));

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
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));
    stateManager.push(MenuState::createMenu(MenuType::PauseMenu));

    // ACT
    stateManager.clear();

    // ASSERT
    REQUIRE(stateManager.isEmpty());
}

TEST_CASE("replaceCurrent replaces the top state")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));
    stateManager.push(MenuState::createMenu(MenuType::PauseMenu));

    // ACT
    stateManager.replaceCurrent(GameplayState::createGameplay());

    // ASSERT
    REQUIRE(typeid(stateManager.getCurrent()) == typeid(GameplayState));
    stateManager.pop();
    REQUIRE(typeid(stateManager.getCurrent()) == typeid(MenuState));
    stateManager.pop();
    REQUIRE(stateManager.isEmpty());
}

TEST_CASE("replaceCurrent on empty state manager throws")
{
    // ARRANGE
    StateManager stateManager;

    // ACT & ASSERT
    REQUIRE_THROWS(stateManager.replaceCurrent(MenuState::createMenu(MenuType::MainMenu)));
}

TEST_CASE("applyAction None does not change current state")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));

    // ACT
    stateManager.applyAction(StateTransitionAction::None);

    // ASSERT
    REQUIRE(typeid(stateManager.getCurrent()) == typeid(MenuState));
}

TEST_CASE("applyAction ReplaceCurrentWithGameplay replaces current state with gameplay")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));

    // ACT
    stateManager.applyAction(StateTransitionAction::ReplaceCurrentWithGameplay);

    // ASSERT
    REQUIRE(typeid(stateManager.getCurrent()) == typeid(GameplayState));
    stateManager.pop();
    REQUIRE(stateManager.isEmpty());
}

TEST_CASE("applyAction ReplaceCurrentWithLoadedGameplay creates gameplay loaded from save")
{
    const auto testDir = createTempTestDirectory();
    ScopedCurrentPath pathGuard(testDir);

    PersistenceManager persistenceManager;
    PersistedGame game;
    game.stage = 9;
    game.wave = 4;
    game.currency = 777;
    game.playerStats.speed = 360.0f;
    persistenceManager.saveGame(game);

    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));

    stateManager.applyAction(StateTransitionAction::ReplaceCurrentWithLoadedGameplay);

    auto *gameplayState = dynamic_cast<GameplayState *>(&stateManager.getCurrent());
    REQUIRE(gameplayState != nullptr);
    REQUIRE(gameplayState->didLoadFromSave());

    const PersistedGame loaded = gameplayState->game.getPersistedGame();
    REQUIRE(loaded.stage == 9);
    REQUIRE(loaded.wave == 4);
    REQUIRE(loaded.currency == 777);
    REQUIRE(loaded.playerStats.speed == 360.0f);

    std::error_code ec;
    fs::remove_all(testDir, ec);
}

TEST_CASE("applyAction PushPauseMenu pushes cancelPressed menu on top")
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
    REQUIRE(typeid(stateManager.getCurrent()) == typeid(ProgressionStoreState));
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
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));
    stateManager.push(MenuState::createMenu(MenuType::PauseMenu));

    // ACT
    stateManager.applyAction(StateTransitionAction::Pop);

    // ASSERT
    REQUIRE(dynamic_cast<MenuState *>(&stateManager.getCurrent())->type == MenuType::MainMenu);
}

TEST_CASE("applyAction ReplaceCurrentWithMainMenu replaces current state with main menu")
{
    // ARRANGE
    StateManager stateManager;
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));

    // ACT
    stateManager.applyAction(StateTransitionAction::ReplaceCurrentWithMainMenu);

    // ASSERT
    REQUIRE(dynamic_cast<MenuState *>(&stateManager.getCurrent())->type == MenuType::MainMenu);
}

TEST_CASE("applyAction ReplaceCurrentWithExitState clears all states and adds exit state")
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