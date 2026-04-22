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

std::unique_ptr<MenuState> MenuState::createMenu(MenuType menuType)
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
    buttons_[prevButtonSelected]->isSelected = false;
    buttons_[selectedButtonID_]->isSelected = true;
    return stateTransAct;
}

void MenuState::initView()
{
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

        auto startGameButton = std::make_shared<Button>();
        startGameButton->width = 300.0f;
        startGameButton->text = text;
        buttons_.emplace_back(startGameButton); // Button ID/Index 0

        Text textQuit;
        textQuit.text = std::string("Quit");

        auto quitButton = std::make_shared<Button>();
        quitButton->text = textQuit;
        quitButton->width = 300.0f;
        quitButton->centerOffsetY = 100;
        buttons_.emplace_back(quitButton); // Button ID/Index 1

        mainMenuCard->items.push_back(title);
        mainMenuCard->items.push_back(startGameButton);
        mainMenuCard->items.push_back(quitButton);
        view_.items.push_back(std::move(mainMenuCard));
        break;
    }
    case MenuType::PauseMenu: {
        Text textResume;
        textResume.text = std::string("Resume");

        auto resumeButton = std::make_shared<Button>();
        resumeButton->text = textResume;
        resumeButton->centerOffsetX = -100;
        buttons_.emplace_back(resumeButton); // Button ID/Index 0

        Text textQuit;
        textQuit.text = std::string("Quit");

        auto quitButton = std::make_shared<Button>();
        quitButton->text = textQuit;
        quitButton->centerOffsetX = 100;
        buttons_.emplace_back(quitButton); // Button ID/Index 1

        std::unique_ptr<Card> pauseCard = std::make_unique<Card>();
        pauseCard->items.push_back(resumeButton);
        pauseCard->items.push_back(quitButton);

        view_.items.push_back(std::move(pauseCard));
        break;
    }
    case MenuType::GameOverMenu:

        Text textGameOver;
        textGameOver.text = std::string("Game Over!");
        textGameOver.centerOffsetY = -100;

        Text textMainMenu;
        textMainMenu.text = std::string("Main Menu");

        auto mainMenuButton = std::make_shared<Button>();
        mainMenuButton->text = textMainMenu;
        mainMenuButton->centerOffsetX = -100;
        buttons_.emplace_back(mainMenuButton); // Button ID/Index 0

        Text textQuit;
        textQuit.text = std::string("Quit");

        auto quitButton = std::make_shared<Button>();
        quitButton->text = textQuit;
        quitButton->centerOffsetX = 100;
        buttons_.emplace_back(quitButton); // Button ID/Index 1

        std::unique_ptr<Card> gameOverCard = std::make_unique<Card>();
        gameOverCard->items.push_back(textGameOver);
        gameOverCard->items.push_back(mainMenuButton);
        gameOverCard->items.push_back(quitButton);

        view_.items.push_back(std::move(gameOverCard));
        break;
    }
    buttons_[selectedButtonID_]->isSelected = true;
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
    debug.gameSession = &game_.getDebugSession();

    if (input.cancelPressed) {
        return controller::StateTransitionAction::PushPauseMenu;
    }

    bool isGameOver = game_.update(input, debug, dt);

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