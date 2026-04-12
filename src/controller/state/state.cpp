#include "controller/state/state.hpp"
#include "controller/view/text.hpp"

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
        if (input.down || input.up) {
            selectedButtonIndex ^= 1;
        }
        if (input.confirm) {
            switch (selectedButtonIndex) {
            case 0:
                return StateTransitionAction::ReplaceCurrentWithGameplay;
                break;
            case 1:
                exit(0);
            }
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
        // TODO delete old Game State
        if (input.left || input.right) {
            selectedButtonIndex ^= 1;
        }
        if (input.confirm) {
            switch (selectedButtonIndex) {
            case 0:
                return StateTransitionAction::ReplaceCurrentWithMainMenu;
                break;
            case 1:
                exit(0);
            }
        }
        break;
    }
    return StateTransitionAction::None;
}

View MenuState::getView()
{
    View view;
    switch (type) {
    case MenuType::MainMenu: {
        std::unique_ptr<Card> mainMenuCard = std::make_unique<Card>();
        mainMenuCard->backgroundColor = {50, 50, 50};
        mainMenuCard->width = 960;
        mainMenuCard->height = 540;

        Text title;
        title.text = std::string("Main Menu");
        title.centerOffsetY = -(mainMenuCard->height / 2 - 10);

        Text text;
        text.text = std::string("Start Game");
        Button startGameButton;
        startGameButton.width = 300.0f;
        startGameButton.text = text;
        startGameButton.isSelected = (selectedButtonIndex == 0);

        Text textQuit;
        textQuit.text = std::string("Quit");
        Button quitButton;
        quitButton.text = textQuit;
        quitButton.width = 300.0f;
        quitButton.centerOffsetY = 100;
        quitButton.isSelected = (selectedButtonIndex == 1);

        mainMenuCard->items.push_back(title);
        mainMenuCard->items.push_back(startGameButton);
        mainMenuCard->items.push_back(quitButton);
        view.items.push_back(std::move(mainMenuCard));
        break;
    }
    case MenuType::PauseMenu: {
        Text textResume;
        textResume.text = std::string("Resume");

        Button resumeButton;
        resumeButton.text = textResume;
        resumeButton.centerOffsetX = -100;
        resumeButton.isSelected = (selectedButtonIndex == 0);

        Text textQuit;
        textQuit.text = std::string("Quit");

        Button quitButton;
        quitButton.text = textQuit;
        quitButton.centerOffsetX = 100;
        quitButton.isSelected = (selectedButtonIndex == 1);

        std::unique_ptr<Card> pauseCard = std::make_unique<Card>();
        pauseCard->items.push_back(resumeButton);
        pauseCard->items.push_back(quitButton);

        view.items.push_back(std::move(pauseCard));
        break;
    }
    case MenuType::GameOverMenu:

        Text TextGameOver;
        TextGameOver.text = std::string("Game Over!");
        TextGameOver.centerOffsetY = -100;

        Text textMainMenu;
        textMainMenu.text = std::string("Main Menu");

        Button mainMenuButton;
        mainMenuButton.text = textMainMenu;
        mainMenuButton.centerOffsetX = -100;
        mainMenuButton.isSelected = (selectedButtonIndex == 0);

        Text textQuit;
        textQuit.text = std::string("Quit");

        Button quitButton;
        quitButton.text = textQuit;
        quitButton.centerOffsetX = 100;
        quitButton.isSelected = (selectedButtonIndex == 1);

        std::unique_ptr<Card> gameOverCard = std::make_unique<Card>();
        gameOverCard->items.push_back(TextGameOver);
        gameOverCard->items.push_back(mainMenuButton);
        gameOverCard->items.push_back(quitButton);

        view.items.push_back(std::move(gameOverCard));
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
    // TODO: Construct view based on gameplay state
    Text text;
    text.text = std::string("<< GAMEPLAY STATE PLACEHOLDER >>\n\n"
                            "- left-mouse-btn -> Progression Store\n"
                            "- enter          -> Game Over\n"
                            "- esc            -> Pause Menu");
    view.items.push_back(text);
    return view;
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