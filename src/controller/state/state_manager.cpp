#include "controller/state/state_manager.hpp"
#include "logging/log.hpp"
#include <exception>
#include <utility>

namespace controller {

namespace {

constexpr const char *kMenuClickSoundPath = "assets/audio/sounds/menu/menu_click.wav";
constexpr const char *kGameMusicPath = "assets/audio/music/game_loop.ogg";
constexpr const char *kGameOverSound = "assets/audio/sounds/character/player_death.wav";

void safePlaySound(audio::AudioController *audioController, const char *path)
{
    if (audioController == nullptr) {
        return;
    }

    try {
        audioController->playSound(path);
    } catch (const std::exception &e) {
        logger::log(logger::WARNING, std::string("StateManager sound playback failed: ") + e.what());
    }
}

void safePlayMusic(audio::AudioController *audioController, const char *path)
{
    if (audioController == nullptr) {
        return;
    }

    try {
        audioController->playMusic(path);
    } catch (const std::exception &e) {
        logger::log(logger::WARNING, std::string("StateManager music playback failed: ") + e.what());
    }
}

void safeStopMusic(audio::AudioController *audioController)
{
    if (audioController == nullptr) {
        return;
    }

    audioController->stopMusic();
}

} // namespace

StateManager::StateManager(audio::AudioController *audioController) : audioController_(audioController) {}

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

void StateManager::applyAction(StateTransitionAction action)
{
    switch (action) {
    case StateTransitionAction::None:
        // No state change
        break;
    case StateTransitionAction::StartNewGameMelee:
        safePlayMusic(audioController_, kGameMusicPath);
        safePlaySound(audioController_, kMenuClickSoundPath);
        replaceCurrent(GameplayState::createNewGameplay(game::CharacterType::Melee));
        break;
    case StateTransitionAction::StartNewGameRanged:
        safePlayMusic(audioController_, kGameMusicPath);
        safePlaySound(audioController_, kMenuClickSoundPath);
        replaceCurrent(GameplayState::createNewGameplay(game::CharacterType::Ranged));
        break;
    case StateTransitionAction::ReplaceCurrentWithLoadedGameplay:
        safePlayMusic(audioController_, kGameMusicPath);
        safePlaySound(audioController_, kMenuClickSoundPath);
        replaceCurrent(GameplayState::createLoadedGameplay());
        break;
    case StateTransitionAction::PushPauseMenu:
        safePlaySound(audioController_, kMenuClickSoundPath);
        push(MenuState::createMenu(MenuType::PauseMenu));
        break;
    case StateTransitionAction::PushProgressionStore:
        push(ProgressionStoreState::createStore());
        break;
    case StateTransitionAction::ReplaceCurrentWithGameOverMenu:
        safeStopMusic(audioController_);
        safePlaySound(audioController_, kGameOverSound);
        replaceCurrent(MenuState::createMenu(MenuType::GameOverMenu));
        break;
    case StateTransitionAction::Pop:
        safePlaySound(audioController_, kMenuClickSoundPath);
        pop();
        break;
    case StateTransitionAction::ReplaceCurrentWithMainMenu:
        safePlaySound(audioController_, kMenuClickSoundPath);
        replaceCurrent(MenuState::createMenu(MenuType::MainMenu));
        break;
    case StateTransitionAction::ReplaceAllStatesWithExit:
        clear();
        push(ExitState::createExitState());
        break;
    case StateTransitionAction::ReplaceCurrentWithCharacterSelection:
        safePlaySound(audioController_, kMenuClickSoundPath);
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