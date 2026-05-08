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
        if (input.downPressed || input.upPressed) {
            selectedButtonId_ ^= 1;
        }
        if (input.cancelPressed) {
            stateTransitionAction = StateTransitionAction::Pop;
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
        if (input.downPressed || input.upPressed) {
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
    for (std::size_t idx = 0; idx < buttons_.size(); idx++) {
        const Button &button = buttons_[idx];
        const bool insideX = std::max(0.0f, input.mouseGridX - button.gridX) <= button.width;
        const bool insideY = std::max(0.0f, input.mouseGridY - button.gridY) <= button.height;

        if (insideX && insideY) {
            return idx;
        }
    }

    return std::nullopt;
}

void MenuState::initView()
{
    // Placeholder for textured background
    Card &backgroundCard = cards_.emplace_back(Card());
    backgroundCard.gridX = 0;
    backgroundCard.gridY = 0;
    backgroundCard.width = gridWidth;
    backgroundCard.height = gridHeight;

    Card &mainMenuCard = cards_.emplace_back(Card());
    mainMenuCard.backgroundColor = {50, 50, 50};

    Text &title = texts_.emplace_back(Text());
    title.gridY = (mainMenuCard.gridY + mainMenuCard.height / 10);

    Button &button1 = buttons_.emplace_back(Button());
    setCenterizedY(button1, getCenterY(mainMenuCard) - button1.height);
    button1.text.gridY = getCenterY(button1);

    Button &button2 = buttons_.emplace_back(Button());
    setCenterizedY(button2, getCenterY(mainMenuCard) + button2.height);
    button2.text.gridY = getCenterY(button2);

    mainMenuCard.items.push_back(title);
    mainMenuCard.items.push_back(button1);
    mainMenuCard.items.push_back(button2);
    backgroundCard.items.push_back(mainMenuCard);
    view_.items.push_back(backgroundCard);

    switch (type) {
    case MenuType::MainMenu: {
        title.text = std::string("Main Menu");
        button1.text.text = std::string("Start Game");
        button2.text.text = std::string("Quit");

        break;
    }
    case MenuType::PauseMenu: {
        title.text = std::string("Paused");
        button1.text.text = std::string("Resume");
        button2.text.text = std::string("Quit");
        break;
    }
    case MenuType::GameOverMenu:
        title.text = std::string("Game Over!");
        title.color = {255, 0, 0};
        button1.text.text = std::string("Main Menu");
        button2.text.text = std::string("Quit");
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