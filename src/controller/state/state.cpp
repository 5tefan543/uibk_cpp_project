#include "controller/state/state.hpp"
#include "controller/view/text.hpp"
#include <iostream>

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
    StateTransitionAction stateTransitionAction = StateTransitionAction::None;
    const size_t prevSelectedButtonId = selectedButtonId_;

    // Only update selected button based on mouse input if mouse was moved or
    // mouse left button was pressed to avoid interfering with keyboard selection
    bool isMouseSelectionActive = input.mouseMoved || input.mouseLeftPressed;

    const std::optional<std::size_t> hoveredButtonId = getHoveredButtonId(input);
    if (isMouseSelectionActive && hoveredButtonId.has_value()) {
        selectedButtonId_ = hoveredButtonId.value();
    }

    const bool isButtonHovered = hoveredButtonId.has_value();
    const bool buttonPressed = input.confirmPressed || (input.mouseLeftPressed && isButtonHovered);

    switch (type) {
    case MenuType::MainMenu:
        if (input.downPressed || input.upPressed) {
            selectedButtonId_ ^= 1;
        }

        if (buttonPressed) {
            switch (selectedButtonId_) {
            case 0:
                stateTransitionAction = StateTransitionAction::ReplaceCurrentWithGameplay;
                break;
            case 1:
                stateTransitionAction = StateTransitionAction::ReplaceAllStatesWithExit;
                break;
            }
        }
        break;

    case MenuType::PauseMenu:
        if (input.leftPressed || input.rightPressed) {
            selectedButtonId_ ^= 1;
        }

        if (buttonPressed) {
            switch (selectedButtonId_) {
            case 0:
                stateTransitionAction = StateTransitionAction::Pop;
                break;
            case 1:
                stateTransitionAction = StateTransitionAction::ReplaceAllStatesWithExit;
                break;
            }
        }
        break;

    case MenuType::GameOverMenu:
        if (input.leftPressed || input.rightPressed) {
            selectedButtonId_ ^= 1;
        }

        if (buttonPressed) {
            switch (selectedButtonId_) {
            case 0:
                stateTransitionAction = StateTransitionAction::ReplaceCurrentWithMainMenu;
                break;
            case 1:
                stateTransitionAction = StateTransitionAction::ReplaceAllStatesWithExit;
                break;
            }
        }
        break;
    }

    buttons_[prevSelectedButtonId].isSelected = false;
    buttons_[selectedButtonId_].isSelected = true;

    return stateTransitionAction;
}

std::optional<std::size_t> MenuState::getHoveredButtonId(const InputState &input) const
{
    // Mouse position conversion is needed since button positions are relative to the center of the screen,
    // but mouse position is relative to the top left corner
    // TODO: refactor to avoid this conversion by using a consistent coordinate system for both buttons and mouse
    // position
    const float mouseXRelativeToCenter = input.mouseX - input.windowWidth / 2.0f;
    const float mouseYRelativeToCenter = input.mouseY - input.windowHeight / 2.0f;

    for (std::size_t idx = 0; idx < buttons_.size(); idx++) {
        const Button &button = buttons_[idx];

        const bool insideX = std::abs(button.centerOffsetX - mouseXRelativeToCenter) <= button.width / 2.0f;
        const bool insideY = std::abs(button.centerOffsetY - mouseYRelativeToCenter) <= button.height / 2.0f;

        if (insideX && insideY) {
            return idx;
        }
    }

    return std::nullopt;
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

        Button &startGameButton = buttons_.emplace_back(Button());
        startGameButton.width = 300.0f;
        startGameButton.text.text = std::string("Start Game");

        Button &quitButton = buttons_.emplace_back(Button());
        quitButton.text.text = std::string("Quit");
        quitButton.width = 300.0f;
        quitButton.centerOffsetY = 100;

        mainMenuCard.items.push_back(title);
        mainMenuCard.items.push_back(startGameButton);
        mainMenuCard.items.push_back(quitButton);
        view_.items.push_back(mainMenuCard);
        break;
    }
    case MenuType::PauseMenu: {
        Card &pauseCard = cards_.emplace_back(Card());

        Text &title = texts_.emplace_back(Text());
        title.text = std::string("Paused");
        title.centerOffsetY = -100;

        Button &resumeButton = buttons_.emplace_back(Button());
        resumeButton.text.text = std::string("Resume");
        resumeButton.centerOffsetX = -100;

        Button &quitButton = buttons_.emplace_back(Button());
        quitButton.text.text = std::string("Quit");
        quitButton.centerOffsetX = 100;

        pauseCard.items.push_back(title);
        pauseCard.items.push_back(resumeButton);
        pauseCard.items.push_back(quitButton);
        view_.items.push_back(pauseCard);
        break;
    }
    case MenuType::GameOverMenu:

        Card &gameOverCard = cards_.emplace_back(Card());

        Text &textGameOver = texts_.emplace_back(Text());
        textGameOver.text = std::string("Game Over!");
        textGameOver.centerOffsetY = -100;

        Button &mainMenuButton = buttons_.emplace_back(Button());
        mainMenuButton.text.text = std::string("Main Menu");
        mainMenuButton.centerOffsetX = -100;

        Button &quitButton = buttons_.emplace_back(Button());
        quitButton.text.text = std::string("Quit");
        quitButton.centerOffsetX = 100;

        gameOverCard.items.push_back(textGameOver);
        gameOverCard.items.push_back(mainMenuButton);
        gameOverCard.items.push_back(quitButton);

        view_.items.push_back(gameOverCard);
        break;
    }

    buttons_[selectedButtonId_].isSelected = true;
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
    return std::make_unique<GameplayState>();
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
    return std::make_unique<ProgressionStoreState>();
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