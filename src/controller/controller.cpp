#include "controller/controller.hpp"
#include <iostream>

namespace controller {

Controller::Controller()
{
    std::cout << "Controller constructed" << std::endl;
    states.push(MenuState::createMainMenu());
}

Controller::~Controller()
{
    std::cout << "Controller destructed" << std::endl;
}

void Controller::update(const InputState &input, float dt)
{
    BaseState &currentState = states.current();
    StateAction action = currentState.update(input, dt);

    switch (action) {
    case StateAction::None:
        // No state change
        break;
    case StateAction::ReplaceWithGameplay:
        states.replaceCurrent(GameplayState::createGameplay());
        break;
    case StateAction::PushPauseMenu:
        states.push(MenuState::createPauseMenu());
        break;
    case StateAction::PushProgressionStore:
        states.push(ProgressionStoreState::createStore());
        break;
    case StateAction::ReplaceWithGameOverMenu:
        states.replaceCurrent(MenuState::createGameOverMenu());
        break;
    case StateAction::Pop:
        states.pop();
        break;
    case StateAction::ReplaceWithMainMenu:
        states.replaceCurrent(MenuState::createMainMenu());
        break;
    default:
        std::cerr << "Unknown state action!" << std::endl;
        break;
    }
}

} // namespace controller
