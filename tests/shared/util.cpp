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

controller::InputState createInputWithMouse(float centerOffsetX, float centerOffsetY)
{
    controller::InputState input;
    input.windowWidth = 1920;
    input.windowHeight = 1080;
    input.mouseX = input.windowWidth / 2.0f + centerOffsetX;
    input.mouseY = input.windowHeight / 2.0f + centerOffsetY;
    return input;
}

controller::StateTransitionAction applyMouseMove(std::unique_ptr<controller::MenuState> &state, float centerOffsetX,
                                                 float centerOffsetY)
{
    controller::InputState input = createInputWithMouse(centerOffsetX, centerOffsetY);
    input.mouseMoved = true;

    controller::DebugContext debug;
    return state->update(input, debug, dummyDeltaTime);
}

controller::StateTransitionAction applyMouseClick(std::unique_ptr<controller::MenuState> &state, float centerOffsetX,
                                                  float centerOffsetY)
{
    controller::InputState input = createInputWithMouse(centerOffsetX, centerOffsetY);
    input.mouseLeftPressed = true;

    controller::DebugContext debug;
    return state->update(input, debug, dummyDeltaTime);
}