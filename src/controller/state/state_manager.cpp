#include <iostream>
#include <utility>

#include "controller/state/state_manager.hpp"

namespace controller {

void StateManager::push(std::unique_ptr<BaseState> state)
{
    states.push_back(std::move(state));
    printDebugInfo();
}

void StateManager::pop()
{
    if (isEmpty()) {
        throw std::runtime_error("StateManager is Empty");
    }
    states.pop_back();
    printDebugInfo();
}

BaseState &StateManager::getCurrent()
{
    if (isEmpty()) {
        throw std::runtime_error("StateManager is Empty");
    }
    return *states.back();
}

bool StateManager::isEmpty() const
{
    return states.empty();
}

void StateManager::clear()
{
    states.clear();
    printDebugInfo();
}

void StateManager::replaceCurrent(std::unique_ptr<BaseState> state)
{
    if (isEmpty()) {
        throw std::runtime_error("StateManager is Empty");
    }
    states.back() = std::move(state);
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
    case StateTransitionAction::ReplaceCurrentWithExitState:
        clear();
        push(ExitState::createExitState());
        break;
    }
}

void StateManager::printDebugInfo() const
{
    std::cout << "States: [";
    for (auto it = states.begin(); it != states.end(); ++it) {

        std::cout << (*it)->toString();

        if (std::next(it) != states.end()) {
            std::cout << " -> ";
        }
    }
    std::cout << "]\n";
}

} // namespace controller