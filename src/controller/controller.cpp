#include "controller/controller.hpp"
#include "controller/debug/debug_context.hpp"
#include "controller/persistence/leaderboard.hpp"
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
    DebugContext &debug = DebugContext::get();

    if (input.toggleDebugPressed) {
        debug.active = !debug.active;
    }

    BaseState &currentState = stateManager_.getCurrent();
    StateTransitionAction action = currentState.update(input, dt);
    stateManager_.applyAction(action);

    if (debug.active) {
        debug.currentStateInfo = stateManager_.getDebugInfo();
    }
}

BaseState &Controller::getCurrentState()
{
    return stateManager_.getCurrent();
}

} // namespace controller
