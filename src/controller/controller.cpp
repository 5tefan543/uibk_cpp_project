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
        debug.active = !debug.active;
    }

    BaseState &currentState = stateManager.getCurrent();
    StateTransitionAction action = currentState.update(input, debug, dt);
    stateManager.applyAction(action);

    if (debug.active) {
        debug.currentStateInfo = stateManager.getDebugInfo();
    }
}

BaseState &Controller::getCurrentState()
{
    return stateManager.getCurrent();
}

DebugState &Controller::getDebugState()
{
    return debug;
}

} // namespace controller
