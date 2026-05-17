#include "controller/state/state.hpp"
#include "controller/debug/debug_context.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "view/text.hpp"
#include <iostream>

namespace controller {

const view::View &BaseState::getView()
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

StateTransitionAction MenuState::update(const InputState &input, [[maybe_unused]] float dt)
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
        if (input.downPressed) {
            selectedButtonId_ = (selectedButtonId_ + 1) % buttons_.size();
        }
        if (input.upPressed) {
            selectedButtonId_ = (selectedButtonId_ + buttons_.size() - 1) % buttons_.size();
        }

        if (buttonPressed) {
            switch (selectedButtonId_) {
            case 0:
                stateTransitionAction = StateTransitionAction::ReplaceCurrentWithGameplay;
                break;
            case 1:
                if (buttons_.size() == 3) {
                    stateTransitionAction = StateTransitionAction::ReplaceCurrentWithLoadedGameplay;
                } else {
                    stateTransitionAction = StateTransitionAction::ReplaceAllStatesWithExit;
                }
                break;
            case 2:
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
        const view::Button &button = buttons_[idx];
        const bool insideX = input.mouseGridX >= button.gridX && input.mouseGridX <= (button.gridX + button.width);
        const bool insideY = input.mouseGridY >= button.gridY && input.mouseGridY <= (button.gridY + button.height);

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
        const bool hasSavedGame = PersistenceManager::hasSavedGame();

        // Placeholder for textured background
        view::Card &backgroundCard = cards_.emplace_back(view::Card());
        backgroundCard.gridX = 0;
        backgroundCard.gridY = 0;
        backgroundCard.width = view::gridWidth;
        backgroundCard.height = view::gridHeight;

        view::Card &mainMenuCard = cards_.emplace_back(view::Card());
        mainMenuCard.backgroundColor = {50, 50, 50};

        view::Text &title = texts_.emplace_back(view::Text());
        title.gridY = (mainMenuCard.gridY + mainMenuCard.height / 10);
        title.text = std::string("Main Menu");

        view::Button &startGameButton = buttons_.emplace_back(view::Button());
        setCenterizedY(startGameButton, getCenterY(mainMenuCard) - startGameButton.height);
        startGameButton.text.gridY = getCenterY(startGameButton);
        startGameButton.text.text = std::string("Start Game");

        if (hasSavedGame) {
            view::Button &loadGameButton = buttons_.emplace_back(view::Button());
            setCenterizedY(loadGameButton, getCenterY(mainMenuCard) + loadGameButton.height / 2);
            loadGameButton.text.text = std::string("Load Game");
            loadGameButton.text.gridY = getCenterY(loadGameButton);
            mainMenuCard.elements.push_back(loadGameButton);
        }

        view::Button &quitButton = buttons_.emplace_back(view::Button());
        int quitButtonYOffset = hasSavedGame ? 2 : 1;
        setCenterizedY(quitButton, getCenterY(mainMenuCard) + quitButtonYOffset * quitButton.height);
        quitButton.text.gridY = getCenterY(quitButton);
        quitButton.text.text = std::string("Quit");

        mainMenuCard.elements.push_back(title);
        mainMenuCard.elements.push_back(startGameButton);
        mainMenuCard.elements.push_back(quitButton);
        backgroundCard.elements.push_back(mainMenuCard);
        view_.nodes.push_back({view::ViewMode::FixedToScreen, backgroundCard});
        break;
    }
    case MenuType::PauseMenu: {
        // Placeholder for textured background
        view::Card &backgroundCard = cards_.emplace_back(view::Card());
        backgroundCard.gridX = 0;
        backgroundCard.gridY = 0;
        backgroundCard.width = view::gridWidth;
        backgroundCard.height = view::gridHeight;

        view::Card &mainMenuCard = cards_.emplace_back(view::Card());
        mainMenuCard.backgroundColor = {50, 50, 50};

        view::Text &title = texts_.emplace_back(view::Text());
        title.gridY = (mainMenuCard.gridY + mainMenuCard.height / 10);
        title.text = std::string("Paused");

        view::Button &resumeButton = buttons_.emplace_back(view::Button());
        setCenterizedY(resumeButton, getCenterY(mainMenuCard) - resumeButton.height);
        resumeButton.text.gridY = getCenterY(resumeButton);
        resumeButton.text.text = std::string("Resume");

        view::Button &quitButton = buttons_.emplace_back(view::Button());
        setCenterizedY(quitButton, getCenterY(mainMenuCard) + quitButton.height);
        quitButton.text.gridY = getCenterY(quitButton);
        quitButton.text.text = std::string("Quit");

        mainMenuCard.elements.push_back(title);
        mainMenuCard.elements.push_back(resumeButton);
        mainMenuCard.elements.push_back(quitButton);
        backgroundCard.elements.push_back(mainMenuCard);
        view_.nodes.push_back({view::ViewMode::FixedToScreen, backgroundCard});
        break;
    }
    case MenuType::GameOverMenu:
        // Placeholder for textured background
        view::Card &backgroundCard = cards_.emplace_back(view::Card());
        backgroundCard.gridX = 0;
        backgroundCard.gridY = 0;
        backgroundCard.width = view::gridWidth;
        backgroundCard.height = view::gridHeight;

        view::Card &mainMenuCard = cards_.emplace_back(view::Card());
        mainMenuCard.backgroundColor = {50, 50, 50};

        view::Text &title = texts_.emplace_back(view::Text());
        title.gridY = (mainMenuCard.gridY + mainMenuCard.height / 10);
        title.text = std::string("Game Over!");
        title.color = {255, 0, 0};

        view::Button &mainMenuButton = buttons_.emplace_back(view::Button());
        setCenterizedY(mainMenuButton, getCenterY(mainMenuCard) - mainMenuButton.height);
        mainMenuButton.text.gridY = getCenterY(mainMenuButton);
        mainMenuButton.text.text = std::string("Main Menu");

        view::Button &quitButton = buttons_.emplace_back(view::Button());
        setCenterizedY(quitButton, getCenterY(mainMenuCard) + quitButton.height);
        quitButton.text.gridY = getCenterY(quitButton);
        quitButton.text.text = std::string("Quit");

        mainMenuCard.elements.push_back(title);
        mainMenuCard.elements.push_back(mainMenuButton);
        mainMenuCard.elements.push_back(quitButton);
        backgroundCard.elements.push_back(mainMenuCard);
        view_.nodes.push_back({view::ViewMode::FixedToScreen, backgroundCard});
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

std::unique_ptr<GameplayState> GameplayState::createNewGameplay()
{
    return std::make_unique<GameplayState>();
}

std::unique_ptr<GameplayState> GameplayState::createLoadedGameplay()
{
    std::optional<PersistedGame> persistedGame = std::nullopt;
    if (PersistenceManager::hasSavedGame()) {
        persistedGame = PersistenceManager::loadGame();
    }

    auto state = std::make_unique<GameplayState>();
    if (persistedGame.has_value()) {
        state->game.loadFromPersistedGame(*persistedGame);
        state->loadedFromSave_ = true;
    }

    return state;
}

bool GameplayState::isLoadedFromPersistedGame() const
{
    return loadedFromSave_;
}

StateTransitionAction GameplayState::update(const InputState &input, float dt)
{
    DebugContext &debug = DebugContext::get();
    debug.gameSession = &game.getDebugSession();

    if (input.cancelPressed) {
        return controller::StateTransitionAction::PushPauseMenu;
    }

    return game.update(input, dt);
}

std::string GameplayState::toString() const
{
    return "Gameplay";
}

const view::View &GameplayState::getView()
{
    game.updateView(view_);
    return view_;
}

std::unique_ptr<ProgressionStoreState> ProgressionStoreState::createStore()
{
    return std::make_unique<ProgressionStoreState>();
}

std::optional<std::size_t> ProgressionStoreState::getHoveredButtonId(const InputState &input) const
{
    for (std::size_t idx = 0; idx < buttons_.size(); idx++) {
        const view::Button &button = buttons_[idx];
        const bool insideX = input.mouseGridX >= button.gridX && input.mouseGridX <= (button.gridX + button.width);
        const bool insideY = input.mouseGridY >= button.gridY && input.mouseGridY <= (button.gridY + button.height);

        if (insideX && insideY) {
            return idx;
        }
    }

    return std::nullopt;
}

void ProgressionStoreState::initView()
{
    view::Card &backgroundCard = cards_.emplace_back(view::Card());
    backgroundCard.gridX = 0;
    backgroundCard.gridY = 0;
    backgroundCard.width = view::gridWidth;
    backgroundCard.height = view::gridHeight;

    view::Card &storeCard = cards_.emplace_back(view::Card());
    storeCard.backgroundColor = {50, 50, 50};

    view::Text &title = texts_.emplace_back(view::Text());
    title.gridY = (storeCard.gridY + storeCard.height / 10);
    title.text = std::string("Store Menu");

    view::Button &startGameButton = buttons_.emplace_back(view::Button());
    setCenterizedY(startGameButton, getCenterY(storeCard) - startGameButton.height);
    startGameButton.text.gridY = getCenterY(startGameButton);
    startGameButton.text.text = std::string("Continue Game");

    view::Button &quitButton = buttons_.emplace_back(view::Button());
    setCenterizedY(quitButton, getCenterY(storeCard) + quitButton.height);
    quitButton.text.gridY = getCenterY(quitButton);
    quitButton.text.text = std::string("Quit Game");

    storeCard.elements.push_back(title);
    storeCard.elements.push_back(startGameButton);
    storeCard.elements.push_back(quitButton);
    backgroundCard.elements.push_back(storeCard);
    view_.nodes.push_back({view::ViewMode::FixedToScreen, backgroundCard});

    buttons_[selectedButtonId_].isSelected = true;
}

ProgressionStoreState::ProgressionStoreState()
{
    initView();
}

StateTransitionAction ProgressionStoreState::update(const InputState &input, [[maybe_unused]] float dt)
{
    const size_t prevSelectedButtonId = selectedButtonId_;

    StateTransitionAction stateTransitionAction = StateTransitionAction::None;
    bool isMouseSelectionActive = input.mouseMoved || input.mouseLeftPressed;

    const std::optional<std::size_t> hoveredButtonId = getHoveredButtonId(input);
    if (isMouseSelectionActive && hoveredButtonId.has_value()) {
        selectedButtonId_ = hoveredButtonId.value();
    }

    const bool isButtonHovered = hoveredButtonId.has_value();
    const bool buttonPressed = input.confirmPressed || (input.mouseLeftPressed && isButtonHovered);

    if (input.downPressed) {
        selectedButtonId_ = (selectedButtonId_ + 1) % buttons_.size();
    }
    if (input.upPressed) {
        selectedButtonId_ = (selectedButtonId_ + buttons_.size() - 1) % buttons_.size();
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

    buttons_[prevSelectedButtonId].isSelected = false;
    buttons_[selectedButtonId_].isSelected = true;

    return stateTransitionAction;
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

std::string ExitState::toString() const
{
    return "ExitState";
}

} // namespace controller