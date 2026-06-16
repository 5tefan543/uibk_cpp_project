#include "controller/state/state_manager.hpp"
#include "logging/log.hpp"
#include <typeinfo>
#include <utility>

namespace controller {

namespace {
// TODO will be integrated into assetconfig after assetmanager
constexpr const char *kMenuClickSoundPath = "assets/audio/sounds/menu/menu_click.wav";
constexpr const char *kMenuHoverSoundPath = "assets/audio/sounds/menu/menu_hover.wav";
constexpr const char *kGameMusicPath = "assets/audio/music/game_loop.ogg";
constexpr const char *kGameOverSound = "assets/audio/sounds/character/player_death.wav";
constexpr const char *kWaveOverSound = "assets/audio/sounds/wave_finished.wav";

} // namespace

StateManager::StateManager() : audioController_(audioCache_) {}

void StateManager::push(std::unique_ptr<BaseState> state)
{
    states_.push_back(std::move(state));
    printDebugInfo();
}

void StateManager::pop()
{
    if (isEmpty()) {
        throw std::runtime_error("StateManager is Empty");
    }
    states_.pop_back();
    printDebugInfo();
}

BaseState &StateManager::getCurrent()
{
    if (isEmpty()) {
        throw std::runtime_error("StateManager is Empty");
    }
    return *states_.back();
}

bool StateManager::isEmpty() const
{
    return states_.empty();
}

void StateManager::clear()
{
    states_.clear();
    printDebugInfo();
}

void StateManager::replaceCurrent(std::unique_ptr<BaseState> state)
{
    if (isEmpty()) {
        throw std::runtime_error("StateManager is Empty");
    }
    states_.back() = std::move(state);
    printDebugInfo();
}

void StateManager::updateAudio()
{
    BaseState &currentState = getCurrent();
    if (auto *menu = dynamic_cast<MenuState *>(&currentState)) {
        if (menu->selectedButtonChanged()) {
            audioController_.playSound(kMenuHoverSoundPath);
        }
    } else if (auto *menu = dynamic_cast<ProgressionStoreState *>(&currentState)) {
        if (menu->selectedButtonChanged()) {
            audioController_.playSound(kMenuHoverSoundPath);
        }
    } else if (auto *menu = dynamic_cast<GameplayState *>(&currentState)) {
        if (menu->hasWaveChanged()) {
            audioController_.playSound(kWaveOverSound);
        }
    }
    audioController_.update();
}

void StateManager::applyAction(StateTransitionAction action)
{
    switch (action) {
    case StateTransitionAction::None:
        // No state change
        break;
    case StateTransitionAction::StartNewGameMelee:
        audioController_.safePlayMusic(kGameMusicPath);
        audioController_.safePlaySound(kMenuClickSoundPath);
        replaceCurrent(GameplayState::createNewGameplay(game::CharacterType::Melee));
        break;
    case StateTransitionAction::StartNewGameRanged:
        audioController_.safePlayMusic(kGameMusicPath);
        audioController_.safePlaySound(kMenuClickSoundPath);
        replaceCurrent(GameplayState::createNewGameplay(game::CharacterType::Ranged));
        break;
    case StateTransitionAction::ReplaceCurrentWithLoadedGameplay:
        audioController_.safePlayMusic(kGameMusicPath);
        audioController_.safePlaySound(kMenuClickSoundPath);
        replaceCurrent(GameplayState::createLoadedGameplay());
        break;
    case StateTransitionAction::PushPauseMenu:
        audioController_.safePauseMusic();
        audioController_.safePlaySound(kMenuClickSoundPath);
        push(MenuState::createMenu(MenuType::PauseMenu));
        break;
    case StateTransitionAction::PushProgressionStore:
        audioController_.safePauseMusic();
        push(ProgressionStoreState::createStore());
        break;
    case StateTransitionAction::ReplaceCurrentWithGameOverMenu:
        audioController_.safeStopMusic();
        audioController_.safePlaySound(kGameOverSound);
        replaceCurrent(MenuState::createMenu(MenuType::GameOverMenu));
        break;
    case StateTransitionAction::Pop:
        audioController_.safeResumeMusic();
        audioController_.safePlaySound(kMenuClickSoundPath);
        pop();
        break;
    case StateTransitionAction::ReplaceCurrentWithMainMenu:
        audioController_.safePlaySound(kMenuClickSoundPath);
        replaceCurrent(MenuState::createMenu(MenuType::MainMenu));
        break;
    case StateTransitionAction::ReplaceAllStatesWithExit:
        clear();
        push(ExitState::createExitState());
        break;
    case StateTransitionAction::ReplaceCurrentWithCharacterSelection:
        audioController_.safePlaySound(kMenuClickSoundPath);
        replaceCurrent(MenuState::createMenu(MenuType::CharacterSelection));
        break;
    }
}

std::string StateManager::getDebugInfo() const
{
    std::string debugInfo = "States: [";
    for (auto it = states_.begin(); it != states_.end(); ++it) {
        debugInfo += (*it)->toString();

        if (std::next(it) != states_.end()) {
            debugInfo += " -> ";
        }
    }
    debugInfo += "]";
    return debugInfo;
}

void StateManager::printDebugInfo() const
{
    logger::log(logger::DEBUG, getDebugInfo());
}

} // namespace controller