#include "shared/util.hpp"

controller::InputState createInputWithMouse(float gridX, float gridY)
{
    controller::InputState input;
    input.mouseGridX = gridX;
    input.mouseGridY = gridY;
    return input;
}
