#include "controller/views/state.hpp"

namespace controller {

std::unique_ptr<MenuState> MenuState::createMainMenu()
{
    auto mainMenu = std::make_unique<MenuState>();
    mainMenu->type = StateType::MainMenu;
    mainMenu->title = "GAME TITLE";
    mainMenu->backgroundColor = {0, 0, 0};
    mainMenu->buttons = {
        {"Start Game", 0.0f, -40.0f, 220.0f, 50.0f},
        {"Quit", 0.0f, 40.0f, 220.0f, 50.0f},
    };
    return mainMenu;
}

std::unique_ptr<MenuState> MenuState::createPauseMenu()
{
    auto pauseMenu = std::make_unique<MenuState>();
    pauseMenu->type = StateType::PauseMenu;
    pauseMenu->title = "PAUSED";
    pauseMenu->backgroundColor = {20, 20, 60};
    pauseMenu->buttons = {
        {"Resume", 0.0f, -40.0f, 220.0f, 50.0f},
        {"Quit", 0.0f, 40.0f, 220.0f, 50.0f},
    };
    return pauseMenu;
}

std::unique_ptr<MenuState> MenuState::createGameOverMenu()
{
    auto gameOverMenu = std::make_unique<MenuState>();
    gameOverMenu->type = StateType::GameOverMenu;
    gameOverMenu->title = "GAME OVER";
    gameOverMenu->backgroundColor = {80, 0, 0};
    gameOverMenu->buttons = {
        {"Back to Main Menu", 0.0f, -40.0f, 300.0f, 50.0f},
    };
    return gameOverMenu;
}

StateAction MenuState::update(const InputState &input, float dt)
{
    switch (type) {
    case StateType::MainMenu:
        if (input.confirm) {
            return StateAction::ReplaceWithGameplay;
        }
        break;

    case StateType::PauseMenu:
        if (input.confirm) {
            return StateAction::Pop;
        }
        break;

    case StateType::GameOverMenu:
        if (input.confirm) {
            return StateAction::ReplaceWithMainMenu;
        }
        break;

    default:
        return StateAction::None;
    }
    return StateAction::None;
}

std::unique_ptr<GameplayState> GameplayState::createGameplay()
{
    auto gameplay = std::make_unique<GameplayState>();
    gameplay->type = StateType::Gameplay;
    gameplay->title = "GAMEPLAY";
    gameplay->backgroundColor = {0, 0, 0};
    // Initialize game and other state variables here
    return gameplay;
}

StateAction GameplayState::update(const InputState &input, float dt)
{
    if (input.pause) {
        return StateAction::PushPauseMenu;
    } else if (input.mouseLeft) {
        return StateAction::PushProgressionStore;
    } else if (input.confirm) {
        return StateAction::ReplaceWithGameOverMenu;
    }
    return StateAction::None;
}

std::unique_ptr<ProgressionStoreState> ProgressionStoreState::createStore()
{
    auto store = std::make_unique<ProgressionStoreState>();
    store->type = StateType::ProgressionStore;
    store->title = "PROGRESSION STORE";
    store->backgroundColor = {0, 0, 0};
    // Initialize store state variables here
    return store;
}

StateAction ProgressionStoreState::update(const InputState &input, float dt)
{
    if (input.confirm) {
        return StateAction::Pop;
    }
    return StateAction::None;
}

} // namespace controller