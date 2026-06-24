#include "controller/state/state_manager.hpp"
#include "logging/log.hpp"
#include <typeinfo>
#include <utility>

namespace controller {

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
            audioController_.playSound(config_.menuSoundConfig.buttonHoverSound);
        }
    } else if (auto *menu = dynamic_cast<ProgressionStoreState *>(&currentState)) {
        if (menu->selectedButtonChanged()) {
            audioController_.playSound(config_.menuSoundConfig.buttonHoverSound);
        }
    } else if (auto *menu = dynamic_cast<GameplayState *>(&currentState)) {
        if (menu->hasWaveChanged()) {
            audioController_.playSound(config_.menuSoundConfig.waveOverSound);
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
        audioController_.playMusic(config_.menuSoundConfig.gameMusic);
        audioController_.playSound(config_.menuSoundConfig.buttonClickSound);
        replaceCurrent(GameplayState::createNewGameplay(game::CharacterType::Melee));
        break;
    case StateTransitionAction::StartNewGameRanged:
        audioController_.playMusic(config_.menuSoundConfig.gameMusic);
        audioController_.playSound(config_.menuSoundConfig.buttonClickSound);
        replaceCurrent(GameplayState::createNewGameplay(game::CharacterType::Ranged));
        break;
    case StateTransitionAction::ReplaceCurrentWithLoadedGameplay:
        audioController_.playMusic(config_.menuSoundConfig.gameMusic);
        audioController_.playSound(config_.menuSoundConfig.buttonClickSound);
        replaceCurrent(GameplayState::createLoadedGameplay());
        break;
    case StateTransitionAction::PushPauseMenu:
        audioController_.pauseMusic();
        audioController_.playSound(config_.menuSoundConfig.buttonClickSound);
        push(MenuState::createMenu(MenuType::PauseMenu));
        break;
    case StateTransitionAction::PushProgressionStore: {
        BaseState &currentState = getCurrent();
        if (auto *gameplayState = dynamic_cast<GameplayState *>(&currentState)) {
            audioController_.pauseMusic();
            push(ProgressionStoreState::createStore(gameplayState->game));
        } else {
            logger::log(logger::ERROR, "Cannot push ProgressionStoreState when current state is not GameplayState.");
        }
        break;
    }
    case StateTransitionAction::ReplaceCurrentWithGameOverMenu:
        audioController_.stopMusic();
        audioController_.playSound(config_.menuSoundConfig.gameOverSound);
        replaceCurrent(MenuState::createMenu(MenuType::GameOverMenu));
        break;
    case StateTransitionAction::Pop:
        audioController_.resumeMusic();
        audioController_.playSound(config_.menuSoundConfig.buttonClickSound);
        pop();
        break;
    case StateTransitionAction::ReplaceCurrentWithMainMenu:
        audioController_.playSound(config_.menuSoundConfig.buttonClickSound);
        replaceCurrent(MenuState::createMenu(MenuType::MainMenu));
        break;
    case StateTransitionAction::ReplaceAllStatesWithExit:
        clear();
        push(ExitState::createExitState());
        break;
    case StateTransitionAction::ReplaceCurrentWithCharacterSelection:
        audioController_.playSound(config_.menuSoundConfig.buttonClickSound);
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