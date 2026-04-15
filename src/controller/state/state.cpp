#include "controller/state/state.hpp"
#include "controller/view/text.hpp"

namespace controller {

std::unique_ptr<MenuState> MenuState::createMenu(MenuType menuType)
{
    auto menu = std::make_unique<MenuState>();
    menu->type = menuType;
    return menu;
}

StateTransitionAction MenuState::update(const InputState &input, [[maybe_unused]] float dt)
{
    switch (type) {
    case MenuType::MainMenu:
        if (input.downPressed || input.upPressed) {
            selectedButtonIndex ^= 1;
        }
        if (input.confirmPressed) {
            switch (selectedButtonIndex) {
            case 0:
                return StateTransitionAction::ReplaceCurrentWithGameplay;
                break;
            case 1:
                return StateTransitionAction::ReplaceAllStatesWithExit;
            }
        }
        break;

    case MenuType::PauseMenu:
        if (input.confirmPressed && selectedButtonIndex == 0) {
            return StateTransitionAction::Pop;
        }
        if (input.leftPressed) {
            selectedButtonIndex = 0;
        } else if (input.rightPressed) {
            selectedButtonIndex = 1;
        }
        break;

    case MenuType::GameOverMenu:
        // TODO delete old Game State
        if (input.leftPressed || input.rightPressed) {
            selectedButtonIndex ^= 1;
        }
        if (input.confirmPressed) {
            switch (selectedButtonIndex) {
            case 0:
                return StateTransitionAction::ReplaceCurrentWithMainMenu;
            case 1:
                return StateTransitionAction::ReplaceAllStatesWithExit;
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
    default:
        return "Unknown (prob. forgot to implement) MenuState";
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

StateTransitionAction ProgressionStoreState::update(const InputState &input, [[maybe_unused]] float dt)
{
    if (input.confirmPressed) {
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

std::unique_ptr<ExitState> ExitState::createExitState()
{
    auto exitState = std::make_unique<ExitState>();
    return exitState;
}

StateTransitionAction ExitState::update([[maybe_unused]] const InputState &input, [[maybe_unused]] float dt)
{
    return StateTransitionAction::ReplaceAllStatesWithExit;
}

View ExitState::getView()
{
    View view;
    // Implement view for exit state,
    return view;
}

std::string ExitState::toString() const
{
    return "ExitState";
}

} // namespace controller
// namespace controller