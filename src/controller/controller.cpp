#include "controller/controller.hpp"
#include "controller/debug/debug_context.hpp"
#include "logging/log.hpp"

namespace controller {

Controller::Controller()
{
    logger::log(logger::DEBUG, "Controller constructed");

    stateManager_.push(MenuState::createMenu(MenuType::MainMenu));
}

Controller::~Controller()
{
    logger::log(logger::DEBUG, "Controller destructed");
}

void Controller::update(const InputState &input, float dt)
{
    stateManager_.updateAudio();

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
