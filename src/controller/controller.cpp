#include "controller/controller.hpp"
#include <iostream>

namespace controller {

Controller::Controller()
{
    std::cout << "Controller constructed" << std::endl;
    stateManager_.push(MenuState::createMenu(MenuType::MainMenu));
}

Controller::~Controller()
{
    std::cout << "Controller destructed" << std::endl;
}

void Controller::update(const InputState &input, float dt)
{
    if (input.toggleDebugPressed) {
        debug_.active = !debug_.active;
    }

    BaseState &currentState = stateManager_.getCurrent();
    StateTransitionAction action = currentState.update(input, debug_, dt);
    stateManager_.applyAction(action);

    if (debug_.active) {
        debug_.currentStateInfo = stateManager_.getDebugInfo();
    }
}

BaseState &Controller::getCurrentState()
{
    return stateManager_.getCurrent();
}

DebugContext &Controller::getDebugContext()
{
    return debug_;
}

} // namespace controller
