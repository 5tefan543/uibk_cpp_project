#include "controller/state/state.hpp"
#include "controller/debug/debug_context.hpp"
#include "controller/input/mouse_util.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "logging/log.hpp"
#include "view/text.hpp"

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
    prevSelectedButtonId_ = selectedButtonId_;

    // Only update selected button based on mouse input if mouse was moved or
    // mouse left button was pressed to avoid interfering with keyboard selection
    bool isMouseSelectionActive = input.mouseMoved || input.mouseLeftPressed;

    const std::optional<std::size_t> hoveredButtonId = MouseUtil::getHoveredButtonId(input, buttons_);
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
                stateTransitionAction = StateTransitionAction::ReplaceCurrentWithCharacterSelection;
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
    case MenuType::CharacterSelection:
        if (input.downPressed || input.upPressed) {
            selectedButtonId_ ^= 1;
        }
        if (buttonPressed) {
            switch (selectedButtonId_) {
            case 0:
                stateTransitionAction = StateTransitionAction::StartNewGameMelee;
                break;
            case 1:
                stateTransitionAction = StateTransitionAction::StartNewGameRanged;
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

    buttons_[prevSelectedButtonId_].isSelected = false;
    buttons_[selectedButtonId_].isSelected = true;

    return stateTransitionAction;
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
    case MenuType::CharacterSelection: {
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
        title.text = std::string("Choose your character!");

        view::Button &meleeButton = buttons_.emplace_back(view::Button());
        setCenterizedY(meleeButton, getCenterY(mainMenuCard) - meleeButton.height);
        meleeButton.text.gridY = getCenterY(meleeButton);
        meleeButton.text.text = std::string("Melee");

        view::Button &rangedButton = buttons_.emplace_back(view::Button());
        setCenterizedY(rangedButton, getCenterY(mainMenuCard) + rangedButton.height);
        rangedButton.text.gridY = getCenterY(rangedButton);
        rangedButton.text.text = std::string("Ranged");

        mainMenuCard.elements.push_back(title);
        mainMenuCard.elements.push_back(meleeButton);
        mainMenuCard.elements.push_back(rangedButton);
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

bool MenuState::selectedButtonChanged()
{
    return selectedButtonId_ != prevSelectedButtonId_;
}

std::string MenuState::toString() const
{
    switch (type) {
    case MenuType::MainMenu:
        return "MainMenu";
    case MenuType::CharacterSelection:
        return "CharacterSelection";
    case MenuType::PauseMenu:
        return "PauseMenu";
    case MenuType::GameOverMenu:
        return "GameOverMenu";
    default:
        std::unreachable();
    }
}

std::unique_ptr<GameplayState> GameplayState::createNewGameplay(const game::CharacterType characterType)
{
    return std::unique_ptr<GameplayState>(new GameplayState(characterType));
}

std::unique_ptr<GameplayState> GameplayState::createLoadedGameplay()
{
    std::optional<game::PersistedGame> persistedGame = std::nullopt;
    if (PersistenceManager::hasSavedGame()) {
        persistedGame = PersistenceManager::loadGame();
    }

    if (persistedGame.has_value()) {
        return std::unique_ptr<GameplayState>(new GameplayState(persistedGame.value()));
    }
    logger::log(logger::WARNING, "No saved game found, starting new game instead.");
    return std::unique_ptr<GameplayState>(new GameplayState(game::CharacterType::Melee));
}

GameplayState::GameplayState(const game::CharacterType characterType) : game(characterType) {}

GameplayState::GameplayState(const game::PersistedGame &persistedGame) : game(persistedGame)
{
    loadedFromSave_ = true;
}

bool GameplayState::isLoadedFromPersistedGame() const
{
    return loadedFromSave_;
}

bool GameplayState::hasWaveChanged()
{
    int gameWave = game.getWaveNumber();
    if (currentWave_ == gameWave)
        return false;
    currentWave_ = gameWave;
    return true;
}

StateTransitionAction GameplayState::update(const InputState &input, float dt)
{
    DebugContext &debug = DebugContext::get();
    debug.gameSession = &game.getDebugSession();

    if (input.cancelPressed) {
        return controller::StateTransitionAction::PushPauseMenu;
    }

    StateTransitionAction action = game.update(input, dt);

    if (game.isGameOver()) {
        debug.gameSession = nullptr;
        return action;
    }

    if (debug.active && debug.gameSession->isStoreOpenRequested) {
        debug.gameSession->isStoreOpenRequested = false;
        return controller::StateTransitionAction::PushProgressionStore;
    }

    return action;
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

ProgressionStoreState::ProgressionStoreState()
{
    initView();
}

std::unique_ptr<ProgressionStoreState> ProgressionStoreState::createStore()
{
    return std::unique_ptr<ProgressionStoreState>(new ProgressionStoreState());
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

StateTransitionAction ProgressionStoreState::update(const InputState &input, [[maybe_unused]] float dt)
{
    prevSelectedButtonId_ = selectedButtonId_;

    StateTransitionAction stateTransitionAction = StateTransitionAction::None;
    bool isMouseSelectionActive = input.mouseMoved || input.mouseLeftPressed;

    const std::optional<std::size_t> hoveredButtonId = MouseUtil::getHoveredButtonId(input, buttons_);
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

    buttons_[prevSelectedButtonId_].isSelected = false;
    buttons_[selectedButtonId_].isSelected = true;

    return stateTransitionAction;
}

bool ProgressionStoreState::selectedButtonChanged()
{
    return selectedButtonId_ != prevSelectedButtonId_;
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