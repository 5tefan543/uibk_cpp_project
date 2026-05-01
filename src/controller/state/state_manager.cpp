#include <iostream>
#include <utility>

#include "controller/state/state_manager.hpp"

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

void StateManager::applyAction(StateTransitionAction action)
{
    switch (action) {
    case StateTransitionAction::None:
        // No state change
        break;
    case StateTransitionAction::ReplaceCurrentWithGameplay:
        replaceCurrent(GameplayState::createGameplay());
        break;
    case StateTransitionAction::ReplaceCurrentWithLoadedGameplay:
        replaceCurrent(GameplayState::createLoadedGameplay());
        break;
    case StateTransitionAction::PushPauseMenu:
        push(MenuState::createMenu(MenuType::PauseMenu));
        break;
    case StateTransitionAction::PushProgressionStore:
        push(ProgressionStoreState::createStore());
        break;
    case StateTransitionAction::ReplaceCurrentWithGameOverMenu:
        replaceCurrent(MenuState::createMenu(MenuType::GameOverMenu));
        break;
    case StateTransitionAction::Pop:
        pop();
        break;
    case StateTransitionAction::ReplaceCurrentWithMainMenu:
        replaceCurrent(MenuState::createMenu(MenuType::MainMenu));
        break;
    case StateTransitionAction::ReplaceAllStatesWithExit:
        clear();
        push(ExitState::createExitState());
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
    std::cout << getDebugInfo() << std::endl;
}

} // namespace controller