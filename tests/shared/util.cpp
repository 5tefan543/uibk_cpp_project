#include "shared/util.hpp"

controller::StateTransitionAction applyInput(std::unique_ptr<controller::MenuState> &state, const INPUT in)
{
    // ARRANGE
    controller::InputState input;
    switch (in) {
    case UP:
        input.upPressed = true;
        break;
    case DOWN:
        input.downPressed = true;
        break;
    case CONFIRM:
        input.confirmPressed = true;
        break;
    case NONE:
        break;
    case LEFT:
        input.leftPressed = true;
        break;
    case RIGHT:
        input.rightPressed = true;
        break;
    }
    controller::DebugContext debug;

    // ACT
    return state->update(input, debug, dummyDeltaTime);
}