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

    // ACT
    return state->update(input, dummyDeltaTime);
}

controller::InputState createInputWithMouse(float gridX, float gridY)
{
    controller::InputState input;
    input.mouseGridX = gridX;
    input.mouseGridY = gridY;
    return input;
}

controller::StateTransitionAction applyMouseMove(std::unique_ptr<controller::MenuState> &state, float gridX,
                                                 float gridY)
{
    controller::InputState input = createInputWithMouse(gridX, gridY);
    input.mouseMoved = true;

    return state->update(input, dummyDeltaTime);
}

controller::StateTransitionAction applyMouseClick(std::unique_ptr<controller::MenuState> &state, float gridX,
                                                  float gridY)
{
    controller::InputState input = createInputWithMouse(gridX, gridY);
    input.mouseLeftPressed = true;

    return state->update(input, dummyDeltaTime);
}