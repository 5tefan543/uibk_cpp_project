#include "controller/state/state.hpp"
#include "controller/view/text.hpp"

namespace controller {

const View &BaseState::getView()
{
    return view_;
}

MenuState::MenuState(MenuType type) : type(type)
{
    initView();
}

std::unique_ptr<MenuState> MenuState::createMenu(const MenuType menuType)
{
    return std::unique_ptr<MenuState>(new MenuState(menuType));
}

StateTransitionAction MenuState::update(const InputState &input, [[maybe_unused]] DebugContext &debug,
                                        [[maybe_unused]] float dt)
{
    StateTransitionAction stateTransAct = StateTransitionAction::None;
    const size_t prevButtonSelected = selectedButtonID_;

    switch (type) {
    case MenuType::MainMenu:
        if (input.downPressed || input.upPressed) {
            selectedButtonID_ ^= 1;
        }
        if (input.confirmPressed) {
            switch (selectedButtonID_) {
            case 0:
                stateTransAct = StateTransitionAction::ReplaceCurrentWithGameplay;
                break;
            case 1:
                stateTransAct = StateTransitionAction::ReplaceAllStatesWithExit;
                break;
            }
        }
        break;

    case MenuType::PauseMenu:
        if (input.confirmPressed && selectedButtonID_ == 0) {
            stateTransAct = StateTransitionAction::Pop;
        }
        if (input.leftPressed) {
            selectedButtonID_ = 0;
        } else if (input.rightPressed) {
            selectedButtonID_ = 1;
        }
        break;

    case MenuType::GameOverMenu:
        // TODO delete old Game State
        if (input.leftPressed || input.rightPressed) {
            selectedButtonID_ ^= 1;
        }
        if (input.confirmPressed) {
            switch (selectedButtonID_) {
            case 0:
                stateTransAct = StateTransitionAction::ReplaceCurrentWithMainMenu;
                break;
            case 1:
                stateTransAct = StateTransitionAction::ReplaceAllStatesWithExit;
                break;
            }
        }
        break;
    }
    buttons_[prevButtonSelected].isSelected = false;
    buttons_[selectedButtonID_].isSelected = true;
    return stateTransAct;
}

void MenuState::initView()
{
    switch (type) {
    case MenuType::MainMenu: {
        Card &mainMenuCard = cards_.emplace_back(Card());
        mainMenuCard.backgroundColor = {50, 50, 50};
        mainMenuCard.width = 960;
        mainMenuCard.height = 540;

        Text &title = texts_.emplace_back(Text());
        title.text = std::string("Main Menu");
        title.centerOffsetY = -(mainMenuCard.height / 2 - 10);

        Button &startGameButton = buttons_.emplace_back(Button()); // Button ID/Index 0
        startGameButton.width = 300.0f;
        startGameButton.text.text = std::string("Start Game");

        Button &quitButton = buttons_.emplace_back(Button()); // Button ID/Index 1
        quitButton.text.text = std::string("Quit");
        quitButton.width = 300.0f;
        quitButton.centerOffsetY = 100;

        mainMenuCard.items.push_back(std::cref(title));
        mainMenuCard.items.push_back(std::cref(startGameButton));
        mainMenuCard.items.push_back(std::cref(quitButton));

        view_.items.push_back(std::cref(mainMenuCard));
        break;
    }
    case MenuType::PauseMenu: {
        Button &resumeButton = buttons_.emplace_back(Button()); // Button ID/Index 0
        resumeButton.text.text = std::string("Resume");
        resumeButton.centerOffsetX = -100;

        Button &quitButton = buttons_.emplace_back(Button()); // Button ID/Index 1
        quitButton.text.text = std::string("Quit");
        quitButton.centerOffsetX = 100;

        Card &pauseCard = cards_.emplace_back(Card());
        pauseCard.items.push_back(resumeButton);
        pauseCard.items.push_back(quitButton);

        view_.items.push_back(pauseCard);
        break;
    }
    case MenuType::GameOverMenu:

        Text &textGameOver = texts_.emplace_back(Text());
        textGameOver.text = std::string("Game Over!");
        textGameOver.centerOffsetY = -100;

        Button &mainMenuButton = buttons_.emplace_back(Button()); // Button ID/Index 0
        mainMenuButton.text.text = std::string("Main Menu");
        mainMenuButton.centerOffsetX = -100;

        Button &quitButton = buttons_.emplace_back(Button()); // Button ID/Index 1
        quitButton.text.text = std::string("Quit");
        quitButton.centerOffsetX = 100;

        Card &gameOverCard = cards_.emplace_back(Card());
        gameOverCard.items.push_back(textGameOver);
        gameOverCard.items.push_back(mainMenuButton);
        gameOverCard.items.push_back(quitButton);

        view_.items.push_back(gameOverCard);
        break;
    }
    buttons_[selectedButtonID_].isSelected = true;
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
        std::unreachable();
    }
}

std::unique_ptr<GameplayState> GameplayState::createGameplay()
{
    auto gameplay = std::make_unique<GameplayState>();
    return gameplay;
}

StateTransitionAction GameplayState::update(const InputState &input, DebugContext &debug, float dt)
{
    debug.gameSession = &game.getDebugSession();

    if (input.cancelPressed) {
        return controller::StateTransitionAction::PushPauseMenu;
    }

    bool isGameOver = game.update(input, debug, dt);

    if (isGameOver) {
        debug.gameSession = nullptr;
        return controller::StateTransitionAction::ReplaceCurrentWithGameOverMenu;
    }

    if (debug.active && debug.gameSession->isStoreOpenRequested) {
        debug.gameSession->isStoreOpenRequested = false;
        return controller::StateTransitionAction::PushProgressionStore;
    }

    return controller::StateTransitionAction::None;
}

std::string GameplayState::toString() const
{
    return "Gameplay";
}

const View &GameplayState::getView()
{
    game.updateView(view_);
    return view_;
}

std::unique_ptr<ProgressionStoreState> ProgressionStoreState::createStore()
{
    auto store = std::make_unique<ProgressionStoreState>();
    return store;
}

StateTransitionAction ProgressionStoreState::update(const InputState &input, [[maybe_unused]] DebugContext &debug,
                                                    [[maybe_unused]] float dt)
{
    if (input.confirmPressed) {
        return StateTransitionAction::Pop;
    }
    return StateTransitionAction::None;
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

StateTransitionAction ExitState::update([[maybe_unused]] const InputState &input, [[maybe_unused]] DebugContext &debug,
                                        [[maybe_unused]] float dt)
{
    return StateTransitionAction::ReplaceAllStatesWithExit;
}

std::string ExitState::toString() const
{
    return "ExitState";
}

} // namespace controller