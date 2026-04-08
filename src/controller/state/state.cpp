#include "controller/state/state.hpp"

namespace controller {

std::unique_ptr<MenuState> MenuState::createMenu(MenuType menuType)
{
    auto menu = std::make_unique<MenuState>();
    menu->type = menuType;
    return menu;
}

StateTransitionAction MenuState::update(const InputState &input, float dt)
{
    switch (type) {
    case MenuType::MainMenu:
        if (input.confirm) {
            return StateTransitionAction::ReplaceCurrentWithGameplay;
        }
        break;

    case MenuType::PauseMenu:
        if (input.confirm && selectedButtonIndex == 0) {
            return StateTransitionAction::Pop;
        }
        if (input.left) {
            selectedButtonIndex = 0;
        } else if (input.right) {
            selectedButtonIndex = 1;
        }
        break;

    case MenuType::GameOverMenu:
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
    case MenuType::MainMenu:
        // TODO: Construct view for main menu
        break;
    case MenuType::PauseMenu: {
        Button resumeButton;
        resumeButton.text = "Resume";
        resumeButton.centerOffsetX = -100;
        resumeButton.isSelected = (selectedButtonIndex == 0);

        Button quitButton;
        quitButton.text = "Quit";
        quitButton.centerOffsetX = 100;
        quitButton.isSelected = (selectedButtonIndex == 1);

        std::unique_ptr<Card> pauseCard = std::make_unique<Card>();
        pauseCard->items.push_back(resumeButton);
        pauseCard->items.push_back(quitButton);

        view.items.push_back(std::move(pauseCard));
        break;
    }
    case MenuType::GameOverMenu:
        // TODO: Construct view for game over menu
        break;
    }
    return view;
}

std::string MenuState::toString() const
{
    switch (type) {
    case MenuType::MainMenu:
        return "MainMenu";
    case MenuType::PauseMenu:
        return "PauseMenu";
    case MenuType::GameOverMenu:
        return "GameOverMenu";
    }
}

std::unique_ptr<GameplayState> GameplayState::createGameplay()
{
    auto gameplay = std::make_unique<GameplayState>();
    return gameplay;
}

StateTransitionAction GameplayState::update(const InputState &input, float dt)
{
    return game.update(input, dt);
}

View GameplayState::getView()
{
    return game.getView();
}

std::string GameplayState::toString() const
{
    return "Gameplay";
}

std::unique_ptr<ProgressionStoreState> ProgressionStoreState::createStore()
{
    auto store = std::make_unique<ProgressionStoreState>();
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
    // TODO: Construct view based on progression store state
    return view;
}

std::string ProgressionStoreState::toString() const
{
    return "ProgressionStore";
}

} // namespace controller