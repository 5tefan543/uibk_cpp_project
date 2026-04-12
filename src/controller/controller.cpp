#include "controller/controller.hpp"
#include <iostream>

namespace controller {

Controller::Controller()
{
    std::cout << "Controller constructed" << std::endl;
    stateManager.push(MenuState::createMenu(MenuType::MainMenu));
}

Controller::~Controller()
{
    std::cout << "Controller destructed" << std::endl;
}

void Controller::update(const InputState &input, float dt)
{
    if (input.toggleDebugPressed) {
        debugState.active = !debugState.active;
    }

    BaseState &currentState = stateManager.getCurrent();
    StateTransitionAction action = currentState.update(input, dt);
    stateManager.applyAction(action);
}

BaseState &Controller::getCurrentState()
{
    return stateManager.getCurrent();
}

DebugState &Controller::getDebugState()
{
    return debugState;
}

} // namespace controller
