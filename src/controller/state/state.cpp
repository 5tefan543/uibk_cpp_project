#include "controller/state/state.hpp"

namespace controller {

std::unique_ptr<MenuState> MenuState::createMainMenu()
{
    auto mainMenu = std::make_unique<MenuState>();
    mainMenu->type = StateType::MainMenu;
    return mainMenu;
}

std::unique_ptr<MenuState> MenuState::createPauseMenu()
{
    auto pauseMenu = std::make_unique<MenuState>();
    pauseMenu->type = StateType::PauseMenu;
    return pauseMenu;
}

std::unique_ptr<MenuState> MenuState::createGameOverMenu()
{
    auto gameOverMenu = std::make_unique<MenuState>();
    gameOverMenu->type = StateType::GameOverMenu;
    return gameOverMenu;
}

StateTransitionAction MenuState::update(const InputState &input, float dt)
{
    switch (type) {
    case StateType::MainMenu:
        if (input.confirm) {
            return StateTransitionAction::ReplaceCurrentWithGameplay;
        }
        break;

    case StateType::PauseMenu:
        if (input.confirm) {
            return StateTransitionAction::Pop;
        }
        break;

    case StateType::GameOverMenu:
        if (input.confirm) {
            return StateTransitionAction::ReplaceCurrentWithMainMenu;
        }
        break;
    }
    return StateTransitionAction::None;
}

View MenuState::getView()
{
    View view;
    switch (type) {
    case StateType::MainMenu:
        // Construct view for main menu
        break;
    case StateType::PauseMenu: {
        // Construct view for pause menu
        Button resumeButton;
        resumeButton.text = "Resume";
        resumeButton.centerOffsetX = -100;
        Button quitButton;
        quitButton.text = "Quit";
        quitButton.centerOffsetX = 100;

        std::unique_ptr<Card> pauseCard = std::make_unique<Card>();
        pauseCard->items.push_back(resumeButton);
        pauseCard->items.push_back(quitButton);

        view.items.push_back(std::move(pauseCard)); // this breaks
        break;
    }
    case StateType::GameOverMenu:
        // Construct view for game over menu
        break;
    }
    return view;
}

std::unique_ptr<GameplayState> GameplayState::createGameplay()
{
    auto gameplay = std::make_unique<GameplayState>();
    gameplay->type = StateType::Gameplay;
    return gameplay;
}

StateTransitionAction GameplayState::update(const InputState &input, float dt)
{
    if (input.pause) {
        return StateTransitionAction::PushPauseMenu;
    } else if (input.mouseLeft) {
        return StateTransitionAction::PushProgressionStore;
    } else if (input.confirm) {
        return StateTransitionAction::ReplaceCurrentWithGameOverMenu;
    }
    return StateTransitionAction::None;
}

View GameplayState::getView()
{
    View view;
    // Construct view based on gameplay state
    return view;
}

std::unique_ptr<ProgressionStoreState> ProgressionStoreState::createStore()
{
    auto store = std::make_unique<ProgressionStoreState>();
    store->type = StateType::ProgressionStore;
    return store;
}

StateTransitionAction ProgressionStoreState::update(const InputState &input, float dt)
{
    if (input.confirm) {
        return StateTransitionAction::Pop;
    }
    return StateTransitionAction::None;
}

View ProgressionStoreState::getView()
{
    View view;
    // Construct view based on progression store state
    return view;
}

} // namespace controller