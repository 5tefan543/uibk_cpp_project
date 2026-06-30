#include "controller/state/state.hpp"
#include "controller/debug/debug_context.hpp"
#include "controller/input/mouse_util.hpp"
#include "controller/persistence/persistence_manager.hpp"
#include "logging/log.hpp"
#include "view/text.hpp"
#include <algorithm>
#include <cctype>

namespace controller {

const view::View &BaseState::getView()
{
    return view_;
}

MenuState::MenuState(MenuType type) : type(type)
{
    initView();
}

MenuState::MenuState(const game::Game &game) : type(MenuType::GameOverMenu)
{
    gameOverData_.emplace();
    GameOverData &gd = *gameOverData_;
    // use persisted snapshot (public API) for score/wave
    const auto persisted = game.getPersistedGame();
    gd.score = persisted.playerStats.score;
    gd.wave = persisted.wave;
    // load top 10 entries
    gd.leaderboardEntries = PersistenceManager::getTopNLeaderboardEntries(10);
    // compute player position among all entries
    LeaderboardEntry temp;
    temp.playerName = std::string();
    temp.score = gd.score;
    temp.wave = gd.wave;
    auto allEntries = PersistenceManager::getLeaderboardEntries();
    std::sort(allEntries.begin(), allEntries.end());
    gd.playerPosition = static_cast<int>(std::count_if(allEntries.begin(), allEntries.end(), [&](const LeaderboardEntry &e) {
        return e < temp;
    })) + 1;

    // reserve name buffer up to 25 chars
    gd.nameBuffer.reserve(25);

    initView();
}

std::unique_ptr<MenuState> MenuState::createMenu(const MenuType menuType)
{
    return std::unique_ptr<MenuState>(new MenuState(menuType));
}

std::unique_ptr<MenuState> MenuState::createMenu(const game::Game &game)
{
    return std::unique_ptr<MenuState>(new MenuState(game));
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
        // If the player hasn't submitted their name yet, accept typed letters/backspace up to 25 chars.
        if (gameOverData_.has_value() && !gameOverData_->nameSubmitted) {
            GameOverData &gd = *gameOverData_;

            // backspace handling
            if (input.backspacePressed && !gd.nameBuffer.empty()) {
                gd.nameBuffer.pop_back();
            }

            // append typed chars (letters only)
            for (char c : input.textEntered) {
                if (gd.nameBuffer.size() >= 25) break;
                if (std::isalpha(static_cast<unsigned char>(c))) {
                    gd.nameBuffer.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
                }
            }

            // update displayed name
            if (gd.nameTextIndex < texts_.size()) {
                texts_[gd.nameTextIndex].text = std::string("Enter name: ") + gd.nameBuffer;
            }

            if (input.confirmPressed) {
                controller::LeaderboardEntry entry;
                entry.playerName = gd.nameBuffer.empty() ? std::string("---") : gd.nameBuffer;
                entry.score = gd.score;
                entry.wave = gd.wave;
                PersistenceManager::storeLeaderboardEntry(entry);

                // refresh leaderboard and player position
                gd.leaderboardEntries = PersistenceManager::getTopNLeaderboardEntries(10);
                auto allEntries = PersistenceManager::getLeaderboardEntries();
                std::sort(allEntries.begin(), allEntries.end());
                gd.playerPosition = static_cast<int>(std::count_if(allEntries.begin(), allEntries.end(), [&](const LeaderboardEntry &e) {
                    LeaderboardEntry temp{entry.playerName, entry.score, entry.wave};
                    return e < temp;
                })) + 1;

                // update leaderboard texts
                for (std::size_t i = 0; i < gd.leaderboardEntries.size(); ++i) {
                    std::size_t idx = gd.leaderboardTextStartIndex + i;
                    if (idx < texts_.size()) {
                        const auto &e = gd.leaderboardEntries[i];
                        texts_[idx].text = std::to_string(i + 1) + std::string(". ") + e.playerName +
                            std::string(" ") + std::to_string(e.score) + std::string(" (W") + std::to_string(e.wave) + std::string(")");
                    }
                }

                // update position text (it's right before nameTextIndex_)
                if (gd.nameTextIndex >= 1) {
                    std::size_t posTextIdx = gd.nameTextIndex - 1;
                    if (posTextIdx < texts_.size()) {
                        texts_[posTextIdx].text = std::string("Your position: ") + (gd.playerPosition > 0 ? std::to_string(gd.playerPosition) : std::string("-"));
                    }
                }

                gd.nameSubmitted = true;
            }
        } else {
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
        backgroundCard.rect = {.position = {0, 0}, .size = view::grid.size};

        view::Card &mainMenuCard = cards_.emplace_back(view::Card());
        mainMenuCard.backgroundColor = {50, 50, 50};
        const auto mainMenuCardCenter = mainMenuCard.rect.getCenter();

        view::Text &title = texts_.emplace_back(view::Text());
        title.position.y = mainMenuCard.rect.position.y + mainMenuCard.rect.size.y / 10;
        title.text = std::string("Main Menu");

        view::Button &startGameButton = buttons_.emplace_back(view::Button());
        startGameButton.rect.centerizeY(mainMenuCardCenter.y - startGameButton.rect.size.y);
        startGameButton.text.position.y = startGameButton.rect.getCenter().y;
        startGameButton.text.text = std::string("Start Game");

        if (hasSavedGame) {
            view::Button &loadGameButton = buttons_.emplace_back(view::Button());
            loadGameButton.rect.centerizeY(mainMenuCardCenter.y + loadGameButton.rect.size.y / 2);
            loadGameButton.text.text = std::string("Load Game");
            loadGameButton.text.position.y = loadGameButton.rect.getCenter().y;
            mainMenuCard.elements.push_back(loadGameButton);
        }

        view::Button &quitButton = buttons_.emplace_back(view::Button());
        int quitButtonYOffset = hasSavedGame ? 2 : 1;
        quitButton.rect.centerizeY(mainMenuCardCenter.y + quitButtonYOffset * quitButton.rect.size.y);
        quitButton.text.position.y = quitButton.rect.getCenter().y;
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
        backgroundCard.rect = view::grid;

        view::Card &mainMenuCard = cards_.emplace_back(view::Card());
        mainMenuCard.backgroundColor = {50, 50, 50};
        const auto mainMenuCardCenter = mainMenuCard.rect.getCenter();

        view::Text &title = texts_.emplace_back(view::Text());
        title.position.y = mainMenuCard.rect.position.y + mainMenuCard.rect.size.y / 10;
        title.text = std::string("Choose your character!");

        view::Button &meleeButton = buttons_.emplace_back(view::Button());
        meleeButton.rect.centerizeY(mainMenuCardCenter.y - meleeButton.rect.size.y);
        meleeButton.text.position.y = meleeButton.rect.getCenter().y;
        meleeButton.text.text = std::string("Melee");

        view::Button &rangedButton = buttons_.emplace_back(view::Button());
        rangedButton.rect.centerizeY(mainMenuCardCenter.y + rangedButton.rect.size.y);
        rangedButton.text.position.y = rangedButton.rect.getCenter().y;
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
        backgroundCard.rect = view::grid;

        view::Card &mainMenuCard = cards_.emplace_back(view::Card());
        mainMenuCard.backgroundColor = {50, 50, 50};
        const auto mainMenuCardCenter = mainMenuCard.rect.getCenter();

        view::Text &title = texts_.emplace_back(view::Text());
        title.position.y = mainMenuCard.rect.position.y + mainMenuCard.rect.size.y / 10;
        title.text = std::string("Paused");

        view::Button &resumeButton = buttons_.emplace_back(view::Button());
        resumeButton.rect.centerizeY(mainMenuCardCenter.y - resumeButton.rect.size.y);
        resumeButton.text.position.y = resumeButton.rect.getCenter().y;
        resumeButton.text.text = std::string("Resume");

        view::Button &quitButton = buttons_.emplace_back(view::Button());
        quitButton.rect.centerizeY(mainMenuCardCenter.y + quitButton.rect.size.y);
        quitButton.text.position.y = quitButton.rect.getCenter().y;
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
        backgroundCard.rect = view::grid;

        view::Card &mainMenuCard = cards_.emplace_back(view::Card());
        mainMenuCard.backgroundColor = {50, 50, 50};
        const auto mainMenuCardCenter = mainMenuCard.rect.getCenter();

        view::Text &title = texts_.emplace_back(view::Text());
        title.position.y = (mainMenuCard.rect.position.y + mainMenuCard.rect.size.y / 10);
        title.text = std::string("Game Over!");
        title.color = {255, 0, 0};
        // show score and wave if available
        if (gameOverData_.has_value()) {
            GameOverData &gd = *gameOverData_;
            view::Text &scoreText = texts_.emplace_back(view::Text());
            scoreText.position.y = title.position.y + 24;
            scoreText.text = std::string("Score: ") + std::to_string(gd.score) +
                std::string("  Wave: ") + std::to_string(gd.wave);
            mainMenuCard.elements.push_back(scoreText);

            // Leaderboard header
            view::Text &lbHeader = texts_.emplace_back(view::Text());
            lbHeader.position.y = scoreText.position.y + 24;
            lbHeader.text = std::string("Leaderboard (Top 10):");
            mainMenuCard.elements.push_back(lbHeader);

            // leaderboard entries
            gd.leaderboardTextStartIndex = texts_.size();
            gd.leaderboardTextCount = gd.leaderboardEntries.size();
            int entryY = lbHeader.position.y + 20;
            for (std::size_t i = 0; i < gd.leaderboardEntries.size(); ++i) {
                const auto &entry = gd.leaderboardEntries[i];
                view::Text &entryText = texts_.emplace_back(view::Text());
                entryText.position.y = entryY + static_cast<int>(i) * 18;
                entryText.text = std::to_string(i + 1) + std::string(". ") + entry.playerName +
                    std::string(" ") + std::to_string(entry.score) + std::string(" (W") + std::to_string(entry.wave) + std::string(")");
                mainMenuCard.elements.push_back(entryText);
            }

            // player's position
            view::Text &posText = texts_.emplace_back(view::Text());
            posText.position.y = entryY + static_cast<int>(gd.leaderboardEntries.size()) * 18 + 8;
            posText.text = std::string("Your position: ") + (gd.playerPosition > 0 ? std::to_string(gd.playerPosition) : std::string("-"));
            mainMenuCard.elements.push_back(posText);

            // name entry prompt
            view::Text &namePrompt = texts_.emplace_back(view::Text());
            namePrompt.position.y = posText.position.y + 22;
            namePrompt.text = std::string("Enter name: ") + gd.nameBuffer;
            gd.nameTextIndex = texts_.size() - 1;
            mainMenuCard.elements.push_back(namePrompt);
        }

        view::Button &mainMenuButton = buttons_.emplace_back(view::Button());
        mainMenuButton.rect.centerizeY(mainMenuCardCenter.y - mainMenuButton.rect.size.y);
        mainMenuButton.text.position.y = mainMenuButton.rect.getCenter().y;
        mainMenuButton.text.text = std::string("Main Menu");

        view::Button &quitButton = buttons_.emplace_back(view::Button());
        quitButton.rect.centerizeY(mainMenuCardCenter.y + quitButton.rect.size.y);
        quitButton.text.position.y = quitButton.rect.getCenter().y;
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
    backgroundCard.rect = view::grid;

    view::Card &storeCard = cards_.emplace_back(view::Card());
    storeCard.backgroundColor = {50, 50, 50};
    const auto storeCardCenter = storeCard.rect.getCenter();

    view::Text &title = texts_.emplace_back(view::Text());
    title.position.y = (storeCard.rect.position.y + storeCard.rect.size.y / 10);
    title.text = std::string("Store Menu");

    view::Button &startGameButton = buttons_.emplace_back(view::Button());
    startGameButton.rect.centerizeY(storeCardCenter.y - startGameButton.rect.size.y);
    startGameButton.text.position.y = startGameButton.rect.getCenter().y;
    startGameButton.text.text = std::string("Continue Game");

    view::Button &quitButton = buttons_.emplace_back(view::Button());
    quitButton.rect.centerizeY(storeCardCenter.y + quitButton.rect.size.y);
    quitButton.text.position.y = quitButton.rect.getCenter().y;
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